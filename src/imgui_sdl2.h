// ImGui GLFW binding with OpenGL3 + shaders
// https://github.com/ocornut/imgui

struct SDL_Window;

bool ImGui_SDL2_Init(SDL_Window* window);
void ImGui_SDL2_Shutdown();
void ImGui_SDL2_NewFrame();

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_SDL2_InvalidateDeviceObjects();
bool ImGui_SDL2_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks.
// You can also handle inputs yourself and use those as a reference.
void ImGui_SDL2_MouseButtonCallback(int button, int action);
void ImGui_SDL2_ScrollCallback(double yoffset);
void ImGui_SDL2_KeyCallback(int key, int scancode, int action, int mods);
void ImGui_SDL2_CharCallback(unsigned int c);
