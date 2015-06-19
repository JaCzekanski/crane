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
	int shadowMapWidth = 1024, shadowMapHeight = 1024;

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
	GLuint depthMapFBO;
	GLuint depthMap;
	glm::vec3 ambient = glm::vec3(0.5, 0.5, 0.5);
	glm::vec3 diffuse = glm::vec3(0.5, 0.5, 0.5);
	glm::vec3 specular = glm::vec3(0.0, 0.0, 0.0);

	// Physics
	bool physicsPaused = true;
	btDiscreteDynamicsWorld* dynamicsWorld;

	// Temporary
	std::string terrainModel = "terrain";
	bool wireframe;
	float fov = glm::radians(60.f);
	glm::vec3 cameraPosition, cameraDirection, cameraUp;
	float cameraPitch, cameraYaw = -3.9f;
	glm::vec3 lightPosition;
	float lightAngle;
	float timer;

	glm::vec3 cranePosition = glm::vec3(0.0, 3.0, 0.0), craneDirection;
	float craneVelocity, craneAcceleration;
	float craneYaw;

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

