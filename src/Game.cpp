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

void Game::Run() 
{
	if (!initialized) return;
	SDL_ShowWindow(mainWindow);
	
	cameraPosition = glm::vec3(0.0f, 15.0f, 45.0f);
	cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	cameraYaw = M_PI;

	focusGained();

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	{
		btCollisionShape* ground = new btBoxShape(btVector3(btScalar(250.), btScalar(10.), btScalar(250.)));

		collisionShapes.push_back(ground);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -10.0, 0));
		btScalar mass(0);

		bool isDynamic = mass != 0.f;

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) ground->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, ground, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	{
		//btTriangleMesh *mesh = new btTriangleMesh();

		//auto model = resourceManager.getModel("cube");
		//for (auto it = model->data.begin(); it != model->data.end(); it+=3)
		//{
		//	glm::vec3 v_[3];
		//	btVector3 v[3];
		//	v_[0] = (*it).position;
		//	v_[1] = (*(it+1)).position;
		//	v_[2] = (*(it+2)).position;

		//	for (int j = 0; j < 3; j++)
		//	{
		//		v[j] = btVector3(v_[j].x, v_[j].y, v_[j].z);
		//	}

		//	mesh->addTriangle(v[0], v[1], v[2]);
		//}		
		//
		//btCollisionShape* shape = new btBvhTriangleMeshShape(mesh, true);//new btBoxShape(btVector3(0.5, 1, 0.5));
		btCollisionShape* shape = new btBoxShape(btVector3(0.5, 1, 0.5));
		collisionShapes.push_back(shape);

		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) shape->calculateLocalInertia(mass, localInertia);

		for (int i = 0; i < 50; i++)
		{
			if (i == 0)
				startTransform.setOrigin(btVector3(0., 50.0, 0.));
			else
				startTransform.setOrigin(btVector3(cos((float)i)*5.0, 150.0 + i*7.0 / sqrt(i), sin((float)i)*5.0));
			startTransform.setRotation(btQuaternion(btVector3(((rand() % 100) / 100.), ((rand() % 100) / 100.), ((rand() % 100) / 100.)), ((rand() % 100) / 100.)));
			btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setCenterOfMassTransform(startTransform);
			body->setRestitution(0.75);

			dynamicsWorld->addRigidBody(body);
		}

	}


	float dt = 0.0f; // Czas od ostatniej klatki
	float lastUpdate = ((float)SDL_GetTicks())/1000.0f; // Czas ostatniej aktualizacji

	float accumulator = 0.0f;
	const float TIME_STEP = 1.f/60.f; // iloœæ aktualizacji fizyki na sekundê (tutaj 30 milisekund, czyli 30x na sekundê)

	float frames = 0;
	float fpsTime = 0;

	while (gameState != State::Quit)
	{
		dt = (((float)SDL_GetTicks()) / 1000.0f) - lastUpdate;
		lastUpdate += dt;
		dt = std::max(0.0f, dt);
		accumulator += dt;

		frames++;
		fpsTime += dt;
		if (fpsTime >= .25f)
		{
			FPS = frames / fpsTime;
			frames = 0;
			fpsTime = 0.f;
		}

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
	if (keys[SDL_SCANCODE_LCTRL]) speed = 100.f;

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
	if (!physicsPaused) dynamicsWorld->stepSimulation(dt);
	lightPosition = glm::vec3(100 * cos(lightAngle), 20.0, 100 * sin(lightAngle));
	lightAngle += dt *0.5;
}

void Game::Render()
{
	gl::Enable(gl::DEPTH_TEST);
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	gl::Disable(gl::CULL_FACE);
	gl::PolygonMode(gl::FRONT_AND_BACK, wireframe ? gl::LINE : gl::FILL);

	gl::ClearColor(1.f, 1.f, 1.f, 1.0f);
	gl::ClearDepth(1.0f);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

	auto program = resourceManager.getProgram("normal");
	program->use();

	glm::mat4 projection = glm::perspective(fov, (float)resolutionWidth / (float)resolutionHeight, 0.1f, 1000.0f);

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
	gl::Uniform3fv(program->getUniform("diffuse"), 1, glm::value_ptr(diffuse));
	gl::Uniform3fv(program->getUniform("ambient"), 1, glm::value_ptr(ambient));
	gl::Uniform3fv(program->getUniform("specular"), 1, glm::value_ptr(specular));
	gl::Uniform1i(program->getUniform("texture"), 0);

	resourceManager.getTexture("bunny")->use();
	for (int i = 0; i < dynamicsWorld->getCollisionObjectArray().size()-1; i++)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1+i];
		btRigidBody* body = btRigidBody::upcast(obj);

		btTransform transform;
		body->getMotionState()->getWorldTransform(transform);

		glm::mat4 model = glm::mat4(1.0);
		glm::quat rotation(transform.getRotation().w(), transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z());
		model = glm::translate(model, glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(), transform.getOrigin().getZ()));
		model = model * glm::mat4_cast(rotation);

		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		resourceManager.getModel("bunny")->render();
	}

	glm::mat4 model = glm::mat4(1.0);
	
	gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
	resourceManager.getTexture("terrain")->use();
	resourceManager.getModel(terrainModel)->render();

	resourceManager.getTexture("skybox")->use();
	resourceManager.getModel("skybox")->render();
	
	
	model = glm::mat4(1.0);
	model = glm::translate(model, cranePosition);
	model = glm::scale(model, glm::vec3(5.f, 5.f, 5.f));
	gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));

	for (auto obj : resourceManager.getModel("crane")->objects)
	{
		if (!obj.second->material.texture.empty())
			resourceManager.getTexture(obj.second->material.texture)->use();

		gl::Uniform3fv(program->getUniform("diffuse"), 1, glm::value_ptr(obj.second->material.diffuse));
		gl::Uniform3fv(program->getUniform("ambient"), 1, glm::value_ptr(obj.second->material.ambient));
		gl::Uniform3fv(program->getUniform("specular"), 1, glm::value_ptr(obj.second->material.specular));

		obj.second->render();
	}

	drawGUI();
	SDL_GL_SwapWindow(mainWindow);
}

void Game::drawGUI()
{
	static bool consoleEnabled = true;
	static bool simpleTerrain = true;
	ImGui_SDL2_NewFrame();
	ImGui::Text("Info");
	ImGui::Text("FPS: %d", (int)FPS);

	ImGui::Text("Options");
	ImGui::Checkbox("Console", &consoleEnabled);
	ImGui::Checkbox("Wireframe mode", &wireframe);
	ImGui::SliderAngle("FOV", &fov, 45, 120);

	if (ImGui::Checkbox("Simple terrain", &simpleTerrain))
	{
		if (simpleTerrain) terrainModel = "simple_terrain";
		else terrainModel = "terrain";
	}

	ImGui::Text("Physics");
	ImGui::Checkbox("Pause", &physicsPaused);

	if (consoleEnabled) {
		ImGui::Begin("Console");
		for (auto m : logger.getMessages()) {
			ImVec4 colors[] = {
				{ 1.0f, 1.0f, 1.0f, 1.0f }, //LOG_INFO = 0,
				{ 0.0f, 1.0f, 0.0f, 1.0f }, //LOG_SUCCESS = 1,
				{ 1.0f, 0.0f, 0.0f, 1.0f }, //LOG_ERROR = 2,
				{ 1.0f, 0.0f, 0.0f, 1.0f }, //LOG_FATAL = 3,
				{ 1.0f, 1.0f, 0.0f, 1.0f }, //LOG_DEBUG = 4
			};

			ImGui::TextColored(colors[m.type], m.message.c_str());
		}
		ImGui::End();
	}
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