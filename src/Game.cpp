#include "Game.h"
#include "gl_core_3_2.hpp"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\euler_angles.hpp>
#include "resource/shader/Program.h"
#include <cmath>
#include <algorithm>
#include <imgui.h>
#include "imgui_sdl2.h"

Game::Game()
{
#ifdef _DEBUG
	logger.Verbose();
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
	{
		logger.Fatal("Cannot initialize SDL");
		return;
	}
	logger.Success("SDL initialized");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	mainWindow = SDL_CreateWindow(
		"Crane",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		resolutionWidth, resolutionHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (mainWindow == NULL)
	{
		logger.Fatal("Cannot create main window");
		return;
	}
	logger.Success("Main window created");

	glcontext = SDL_GL_CreateContext(mainWindow);
	if (glcontext == NULL)
	{
		logger.Fatal("Cannot create OpenGL context");
		return;
	}
	logger.Success("OpenGL context created");

	if (!gl::sys::LoadFunctions())
	{
		logger.Fatal("Cannot load OpenGL functions");
		return;
	}
	logger.Success("OpenGL functions loaded");

	SDL_GL_SetSwapInterval(1); // VSync
	logger.Info("Graphics card: %s", gl::GetString(gl::RENDERER));
	logger.Info("OpenGL version: %d.%d", gl::sys::GetMajorVersion(), gl::sys::GetMinorVersion());


	ImGui_SDL2_Init(mainWindow);
	initialized = true;
}


Game::~Game() 
{
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

int currentModel = 0;
const char* models[] = {
	"cube",
	"bunny"
};
int modelCount = 2;

void Game::Run() 
{
	if (!initialized) return;
	SDL_ShowWindow(mainWindow);
	
	gl::GenVertexArrays(1, &vao);
	gl::BindVertexArray(vao);
	
	auto program = resourceManager.getProgram("normal");
	gl::EnableVertexAttribArray(program->getAttrib("position"));
	gl::EnableVertexAttribArray(program->getAttrib("normal"));
	gl::EnableVertexAttribArray(program->getAttrib("texcoord"));

	cameraPosition = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	cameraYaw = M_PI;

	focusGained();

	float dt = 0.0f; // Czas od ostatniej klatki
	float lastUpdate = ((float)SDL_GetTicks())/1000.0f; // Czas ostatniej aktualizacji

	float accumulator = 0.0f;
	const float TIME_STEP = 0.03f; // ilo�� aktualizacji fizyki na sekund� (tutaj 30 milisekund, czyli 30x na sekund�)

	while (gameState != State::Quit)
	{
		dt = (((float)SDL_GetTicks()) / 1000.0f) - lastUpdate;
		lastUpdate += dt;
		dt = std::max(0.0f, dt);
		accumulator += dt;

		resourceManager.scanAndReload();
		Input(dt);
		while (accumulator > TIME_STEP)
		{
			Step(TIME_STEP);
			accumulator -= TIME_STEP;
		}
		Render();
		SDL_Delay(1);
	}

}

void Game::Input(float dt)
{
	float speed = 20.f;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)	gameState = State::Quit;
		else if (event.type == SDL_KEYDOWN)
		{
			ImGui_SDL2_KeyCallback(event.key.keysym.sym, event.key.keysym.scancode, event.key.state, event.key.keysym.mod);
			keyDown(event.key.keysym.sym);
		}
		else if (event.type == SDL_KEYUP)
		{
			ImGui_SDL2_KeyCallback(event.key.keysym.sym, event.key.keysym.scancode, event.key.state, event.key.keysym.mod);
			keyUp(event.key.keysym.sym);
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				resolutionWidth = event.window.data1;
				resolutionHeight = event.window.data2;
				gl::Viewport(0, 0, resolutionWidth, resolutionHeight);
			}
			if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) focusLost();
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
		{
			ImGui_SDL2_MouseButtonCallback(event.button.button, event.button.state);
			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONDOWN && event.button.clicks == 2) focusGained();
		}
		else if (event.type == SDL_MOUSEMOTION && isWindowFocused) mouseMove(event.motion.xrel, event.motion.yrel);
		else if (event.type == SDL_MOUSEWHEEL)
		{
			ImGui_SDL2_ScrollCallback(event.wheel.y);
			if (isWindowFocused) cameraPosition.y += event.wheel.y * dt * speed;
		}
	}
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LSHIFT]) speed = 10.f;

	if (keys[SDL_SCANCODE_W])
		cameraPosition += cameraDirection * dt * speed;
	if (keys[SDL_SCANCODE_S])
		cameraPosition -= cameraDirection * dt * speed;
	if (keys[SDL_SCANCODE_A])
		cameraPosition -= glm::cross(glm::vec3(sin(cameraYaw), 0, cos(cameraYaw)), cameraUp) * dt * speed;
	if (keys[SDL_SCANCODE_D])
		cameraPosition += glm::cross(glm::vec3(sin(cameraYaw), 0, cos(cameraYaw)), cameraUp) * dt * speed;

	if (keys[SDL_SCANCODE_Q])
		cameraPosition.y += dt * speed;
	if (keys[SDL_SCANCODE_Z])
		cameraPosition.y -= dt * speed;
}


void Game::Step(float dt)
{
	lightPosition = glm::vec3(20 * cos(lightAngle), 0.0, 20 * sin(lightAngle));
}

bool wireframe;
float fov = glm::radians(60.f);

void Game::Render()
{
	gl::Enable(gl::DEPTH_TEST);
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	//gl::Disable(gl::CULL_FACE);
	//gl::CullFace(gl::BACK);
	gl::PolygonMode(gl::FRONT_AND_BACK, wireframe ? gl::LINE : gl::FILL);

	gl::ClearColor(1.f, 1.f, 1.f, 1.0f);
	gl::ClearDepth(1.0f);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

	gl::BindVertexArray(vao);

	auto program = resourceManager.getProgram("normal");
	program->use();

	auto texture = resourceManager.getTexture(models[currentModel]);
	texture->use();

	auto mesh = resourceManager.getModel(models[currentModel]);
	mesh->use();

	gl::VertexAttribPointer(program->getAttrib("position"), 3, gl::FLOAT, false, sizeof(modelVertice), 0);
	gl::VertexAttribPointer(program->getAttrib("normal"), 3, gl::FLOAT, false, sizeof(modelVertice), (void*)(3 * sizeof(GLfloat)));
	gl::VertexAttribPointer(program->getAttrib("texcoord"), 2, gl::FLOAT, false, sizeof(modelVertice), (void*)(6 * sizeof(GLfloat)));
	glm::mat4 projection = glm::perspective(fov, (float)resolutionWidth / (float)resolutionHeight, 0.1f, 10000.0f);

	cameraDirection = glm::vec3(
		cos(cameraPitch) * sin(cameraYaw),
		sin(cameraPitch),
		cos(cameraPitch) * cos(cameraYaw));

	glm::mat4 view = glm::lookAt(
		cameraPosition,
		cameraPosition + cameraDirection,
		cameraUp);

	gl::UniformMatrix4fv(program->getUniform("proj"), 1, false, glm::value_ptr(projection));
	gl::UniformMatrix4fv(program->getUniform("view"), 1, false, glm::value_ptr(view));
	gl::Uniform3fv(program->getUniform("lightPosition"), 1, glm::value_ptr(lightPosition));
	gl::Uniform3fv(program->getUniform("cameraPosition"), 1, glm::value_ptr(cameraPosition));
	gl::Uniform1i(program->getUniform("texture"), 0);

	glm::mat4 model = glm::mat4(1.0);
	gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
	gl::DrawArrays(gl::TRIANGLES, 0, mesh->getSize());


	drawGUI();
	SDL_GL_SwapWindow(mainWindow);
}

void Game::drawGUI()
{
	ImGui_SDL2_NewFrame();
	ImGui::Text("Object");
	ImGui::Combo("Model", &currentModel, models, modelCount);

	ImGui::Text("Options");
	ImGui::Checkbox("Wireframe mode", &wireframe);
	ImGui::SliderAngle("FOV", &fov, 45, 120);

	ImGui::Begin("Console");
	for (auto m : logger.getMessages()) {
		ImVec4 colors[] = {
			{ 1.0f, 1.0f, 1.0f, 1.0f }, //LOG_INFO = 0,
			{ 0.0f, 1.0f, 0.0f, 1.0f }, //LOG_SUCCESS = 1,
			{ 0.7f, 0.3f, 0.3f, 1.0f }, //LOG_ERROR = 2,
			{ 1.0f, 0.0f, 0.0f, 1.0f }, //LOG_FATAL = 3,
			{ 1.0f, 1.0f, 0.0f, 1.0f }, //LOG_DEBUG = 4
		};

		ImGui::TextColored(colors[m.type], m.message.c_str());
	}
	ImGui::End();
	ImGui::Render();
}


void Game::keyDown(SDL_Keycode key)
{
	if (key == SDLK_ESCAPE)
	{
		if (isWindowFocused) focusLost();
		else gameState = State::Quit;
	}
}

void Game::keyUp(SDL_Keycode key)
{

}

void Game::mouseMove(int xrel, int yrel)
{
	const float mouseSpeed = 75.f;
	float dt = 0.025f;
	cameraPitch -= ((float)yrel / resolutionHeight) * dt * mouseSpeed;
	cameraPitch = fmax(fmin(cameraPitch, (M_PI / 2) - 0.0001f), (-M_PI / 2) + 0.0001f);

	cameraYaw -= ((float)xrel / resolutionWidth) * dt * mouseSpeed;
}


void Game::mouseScroll(int yscroll)
{
	
}

void Game::focusLost()
{
	if (!isWindowFocused) return;
	logger.Debug("Focus lost");
	isWindowFocused = false;
	SDL_SetRelativeMouseMode((SDL_bool)false);
}

void Game::focusGained()
{
	if (isWindowFocused) return;
	logger.Debug("Focus gained");
	isWindowFocused = true;
	SDL_SetRelativeMouseMode((SDL_bool)true);
}