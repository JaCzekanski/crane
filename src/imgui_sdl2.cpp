// ImGui GLFW binding with OpenGL3 + shaders
// https://github.com/ocornut/imgui

#include <imgui.h>
#include "imgui_sdl2.h"

// GL3W/GLFW
#include "gl_core_3_2.hpp"
#include <SDL.h>
#include <SDL_syswm.h>

// Data
static SDL_Window*  g_window;
static double       g_Time = 0.0f;
static bool         g_MousePressed[3] = { false, false, false };
static float        g_MouseWheel = 0.0f;
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static size_t       g_VboMaxSize = 20000;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0;

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
static void ImGui_SDL2_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    if (cmd_lists_count == 0)
        return;

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    gl::Enable(gl::BLEND);
    gl::BlendEquation(gl::FUNC_ADD);
    gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
    gl::Disable(gl::CULL_FACE);
    gl::Disable(gl::DEPTH_TEST);
    gl::Enable(gl::SCISSOR_TEST);
	gl::ActiveTexture(gl::TEXTURE0);
	gl::PolygonMode(gl::FRONT_AND_BACK, gl::FILL);

    // Setup orthographic projection matrix
    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;
    const float ortho_projection[4][4] =
    {
        { 2.0f/width,	0.0f,			0.0f,		0.0f },
        { 0.0f,			2.0f/-height,	0.0f,		0.0f },
        { 0.0f,			0.0f,			-1.0f,		0.0f },
        { -1.0f,		1.0f,			0.0f,		1.0f },
    };
    gl::UseProgram(g_ShaderHandle);
    gl::Uniform1i(g_AttribLocationTex, 0);
    gl::UniformMatrix4fv(g_AttribLocationProjMtx, 1, false, &ortho_projection[0][0]);

    // Grow our buffer according to what we need
    size_t total_vtx_count = 0;
    for (int n = 0; n < cmd_lists_count; n++)
        total_vtx_count += cmd_lists[n]->vtx_buffer.size();
    gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
    size_t neededBufferSize = total_vtx_count * sizeof(ImDrawVert);
    if (neededBufferSize > g_VboMaxSize)
    {
        g_VboMaxSize = neededBufferSize + 5000;  // Grow buffer
        gl::BufferData(gl::ARRAY_BUFFER, g_VboMaxSize, NULL, gl::STREAM_DRAW);
    }

    // Copy and convert all vertices into a single contiguous buffer
    unsigned char* buffer_data = (unsigned char*)gl::MapBuffer(gl::ARRAY_BUFFER, gl::WRITE_ONLY);
    if (!buffer_data)
        return;
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        memcpy(buffer_data, &cmd_list->vtx_buffer[0], cmd_list->vtx_buffer.size() * sizeof(ImDrawVert));
        buffer_data += cmd_list->vtx_buffer.size() * sizeof(ImDrawVert);
    }
    gl::UnmapBuffer(gl::ARRAY_BUFFER);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);
    gl::BindVertexArray(g_VaoHandle);

    int cmd_offset = 0;
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        int vtx_offset = cmd_offset;
        const ImDrawCmd* pcmd_end = cmd_list->commands.end();
        for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
        {
            if (pcmd->user_callback)
            {
                pcmd->user_callback(cmd_list, pcmd);
            }
            else
            {
                gl::BindTexture(gl::TEXTURE_2D, (GLuint)(intptr_t)pcmd->texture_id);
                gl::Scissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
                gl::DrawArrays(gl::TRIANGLES, vtx_offset, pcmd->vtx_count);
            }
            vtx_offset += pcmd->vtx_count;
        }
        cmd_offset = vtx_offset;
    }

    // Restore modified state
    gl::BindVertexArray(0);
    gl::UseProgram(0);
    gl::Disable(gl::SCISSOR_TEST);
    gl::BindTexture(gl::TEXTURE_2D, 0);
}

static const char* ImGui_SDL2_GetClipboardText()
{
	return SDL_GetClipboardText();
}

static void ImGui_SDL2_SetClipboardText(const char* text)
{
	SDL_SetClipboardText(text);
}

void ImGui_SDL2_MouseButtonCallback(int button, int action)
{
    if (action == SDL_PRESSED && button >= 1 && button < 3)
        g_MousePressed[button] = true;
}

void ImGui_SDL2_ScrollCallback(double yoffset)
{
    g_MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void ImGui_SDL2_KeyCallback(int key, int, int action, int mods)
{
	key &= 0xffff;
    ImGuiIO& io = ImGui::GetIO();
    if (action == SDL_PRESSED)
        io.KeysDown[key] = true;
    if (action == SDL_RELEASED)
        io.KeysDown[key] = false;
	io.KeyCtrl = (mods & KMOD_CTRL) != 0;
	io.KeyShift = (mods & KMOD_SHIFT) != 0;
	io.KeyAlt = (mods & KMOD_ALT) != 0;
}

void ImGui_SDL2_CharCallback(unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void ImGui_SDL2_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    gl::GenTextures(1, &g_FontTexture);
    gl::BindTexture(gl::TEXTURE_2D, g_FontTexture);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, width, height, 0, gl::RGBA, gl::UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;
}

bool ImGui_SDL2_CreateDeviceObjects()
{
    const GLchar *vertex_shader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    g_ShaderHandle = gl::CreateProgram();
    g_VertHandle = gl::CreateShader(gl::VERTEX_SHADER);
    g_FragHandle = gl::CreateShader(gl::FRAGMENT_SHADER);
    gl::ShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    gl::ShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    gl::CompileShader(g_VertHandle);
    gl::CompileShader(g_FragHandle);
    gl::AttachShader(g_ShaderHandle, g_VertHandle);
    gl::AttachShader(g_ShaderHandle, g_FragHandle);
    gl::LinkProgram(g_ShaderHandle);

    g_AttribLocationTex = gl::GetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = gl::GetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = gl::GetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = gl::GetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = gl::GetAttribLocation(g_ShaderHandle, "Color");

    gl::GenBuffers(1, &g_VboHandle);
    gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
    gl::BufferData(gl::ARRAY_BUFFER, g_VboMaxSize, NULL, gl::DYNAMIC_DRAW);

    gl::GenVertexArrays(1, &g_VaoHandle);
    gl::BindVertexArray(g_VaoHandle);
    gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
    gl::EnableVertexAttribArray(g_AttribLocationPosition);
    gl::EnableVertexAttribArray(g_AttribLocationUV);
    gl::EnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    gl::VertexAttribPointer(g_AttribLocationPosition, 2, gl::FLOAT, gl::FALSE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    gl::VertexAttribPointer(g_AttribLocationUV, 2, gl::FLOAT, gl::FALSE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    gl::VertexAttribPointer(g_AttribLocationColor, 4, gl::UNSIGNED_BYTE, gl::TRUE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF
    gl::BindVertexArray(0);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);

    ImGui_SDL2_CreateFontsTexture();

    return true;
}

bool ImGui_SDL2_Init(SDL_Window* window)
{
	g_window = window;
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                 // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDLK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDLK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDLK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDLK_DOWN;
    io.KeyMap[ImGuiKey_Home] = SDLK_HOME;
    io.KeyMap[ImGuiKey_End] = SDLK_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.RenderDrawListsFn = ImGui_SDL2_RenderDrawLists;
    io.SetClipboardTextFn = ImGui_SDL2_SetClipboardText;
    io.GetClipboardTextFn = ImGui_SDL2_GetClipboardText;
#ifdef _MSC_VER
	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(g_window, &info);
	io.ImeWindowHandle = info.info.win.window;
#endif

    return true;
}

void ImGui_SDL2_Shutdown()
{
    if (g_VaoHandle) gl::DeleteVertexArrays(1, &g_VaoHandle);
    if (g_VboHandle) gl::DeleteBuffers(1, &g_VboHandle);
    g_VaoHandle = 0;
    g_VboHandle = 0;

    gl::DetachShader(g_ShaderHandle, g_VertHandle);
    gl::DeleteShader(g_VertHandle);
    g_VertHandle = 0;

    gl::DetachShader(g_ShaderHandle, g_FragHandle);
    gl::DeleteShader(g_FragHandle);
    g_FragHandle = 0;

    gl::DeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    if (g_FontTexture)
    {
        gl::DeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
    ImGui::Shutdown();
}

void ImGui_SDL2_NewFrame()
{
    if (!g_FontTexture)
        ImGui_SDL2_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
	SDL_GetWindowSize(g_window, &w, &h);
    //glfwGetWindowSize(g_Window, &w, &h);
    //glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);

    // Setup time step
    double current_time =  ((double)SDL_GetTicks())/1000.0;
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
	if (io.DeltaTime < 0.0001f) io.DeltaTime = 0.0001;
    g_Time = current_time;

    // Setup inputs4
	
    if (1)//glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
    {
    	int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
    	io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
    	io.MousePos = ImVec2(-1,-1);
    }

    for (int i = 0; i < 3; i++)
    {
		io.MouseDown[i] = g_MousePressed[i] || ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(i+1)) != 0); // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        g_MousePressed[i] = false;
    }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide/show hardware mouse cursor
	//SDL_ShowCursor(io.MouseDrawCursor ? 1 : 0);

    // Start the frame
    ImGui::NewFrame();
}
