#pragma once
#include <iostream>
#include <cmath>
#include <memory>

#include <SDL.h>

#include <glm\glm.hpp>

#include "Logger.h"
#include "resource/ResourceManager.h"
#include "utils/physics.h"

#include "objects\Camera.h"
#include "objects\Crane.h"

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
	const std::string shadowmapShader = "shadow";
	const std::string normalShader = "normal";


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
	bool physicsPaused = false;
	bool viewPhysics = false;
	btDiscreteDynamicsWorld* dynamicsWorld;

	// Objects
	Camera camera;
	Crane crane;
	std::vector<btRigidBody*> wall;

	// Temporary
	bool consoleEnabled = false;
	bool showHelp = true;
	std::string terrainModel = "terrain";
	bool wireframe;
	float fov = glm::radians(60.f);
	glm::vec3 lightPosition;
	float lightAngle;

	// Setup
	void initializePhysics();
	void generateBrickWall(int xCount = 1, int yCount = 10, int zCount = 10);
	void createShadowmap();

	void Input(float dt);
	void Step(float dt);
	void Render();
	void beginNormalRender();
	void beginShadowmapRender();
	void renderScene(std::string shader);
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

