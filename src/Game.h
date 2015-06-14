#pragma once
#include <iostream>
#include <cmath>
#include <memory>

#include <SDL.h>

#include <glm\glm.hpp>

#include "Logger.h"
#include "resource/ResourceManager.h"
#include <btBulletDynamicsCommon.h>

namespace State
{
	enum State
	{
		Quit = 0,
		Menu,
		Pause,
		Play, //?
	};
};


class Game
{
private:
	int resolutionWidth = 800, resolutionHeight = 600;
	
	ResourceManager resourceManager;

	// SDL
	SDL_Window *mainWindow;
	bool isWindowFocused;

	// OpenGL
	SDL_GLContext glcontext;

	State::State gameState = State::Menu;
	bool initialized = false;

	float FPS = 0;

	// Light
	glm::vec3 diffuse = glm::vec3(0.75, 0.75, 0.75);
	glm::vec3 ambient = glm::vec3(0.75, 0.75, 0.75);
	glm::vec3 specular = glm::vec3(0.0, 0.0, 0.0);

	// Physics
	bool physicsPaused = true;
	btDiscreteDynamicsWorld* dynamicsWorld;

	// Temporary
	std::string terrainModel = "simple_terrain";
	bool wireframe;
	float fov = glm::radians(60.f);
	glm::vec3 cameraPosition, cameraDirection, cameraUp;
	float cameraPitch, cameraYaw = -3.9;
	glm::vec3 lightPosition;
	float lightAngle;

	glm::vec3 cranePosition = glm::vec3(15.f, 0.f, -15.f);

	void Input(float dt);
	void Step(float dt);
	void Render();
	void drawGUI();

	void keyDown(SDL_Keycode key);
	void keyUp(SDL_Keycode key);
	void mouseMove(int xrel, int yrel);
	void mouseScroll(int yscroll);

	void focusLost();
	void focusGained();

public:
	Game();
	~Game();

	void Run();
};

