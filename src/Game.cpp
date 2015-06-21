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

	initializePhysics();
	createShadowmap();

	crane.createPhysicsModel(dynamicsWorld);

	ImGui_SDL2_Init(mainWindow);
	initialized = true;
}


Game::~Game() 
{
	// TODO: No opengl cleaning
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}


void Game::Run() 
{
	if (!initialized) return;
	SDL_ShowWindow(mainWindow);
	focusGained();

	const float TIME_STEP = 1.f / 60.f; // Physics timestep

	float dt = 0.0f;
	float lastUpdate = ((float)SDL_GetTicks())/1000.0f;
	float accumulator = 0.0f;
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

void Game::initializePhysics()
{
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	{
		btTriangleMesh *mesh = new btTriangleMesh();

		auto model = resourceManager.getModel(terrainModel)->objects.begin()->second;
		for (auto it = model->data.begin(); it != model->data.end(); it += 3)
		{
			glm::vec3 v_[3];
			btVector3 v[3];
			v_[0] = (*it).position;
			v_[1] = (*(it + 1)).position;
			v_[2] = (*(it + 2)).position;

			for (int j = 0; j < 3; j++)
			{
				v[j] = btVector3(v_[j].x, v_[j].y, v_[j].z);
			}

			mesh->addTriangle(v[0], v[1], v[2]);
		}

		btCollisionShape* ground = new btBvhTriangleMeshShape(mesh, true);

		collisionShapes.push_back(ground);

		btTransform tr;
		tr.setIdentity();
		btRigidBody* body = createRigidBody(0, tr, ground);
		dynamicsWorld->addRigidBody(body);
	}

	//{
	//	//btTriangleMesh *mesh = new btTriangleMesh();

	//	//auto model = resourceManager.getModel("cube");
	//	//for (auto it = model->data.begin(); it != model->data.end(); it+=3)
	//	//{
	//	//	glm::vec3 v_[3];
	//	//	btVector3 v[3];
	//	//	v_[0] = (*it).position;
	//	//	v_[1] = (*(it+1)).position;
	//	//	v_[2] = (*(it+2)).position;

	//	//	for (int j = 0; j < 3; j++)
	//	//	{
	//	//		v[j] = btVector3(v_[j].x, v_[j].y, v_[j].z);
	//	//	}

	//	//	mesh->addTriangle(v[0], v[1], v[2]);
	//	//}		
	//	//
	//	//btCollisionShape* shape = new btBvhTriangleMeshShape(mesh, true);//new btBoxShape(btVector3(0.5, 1, 0.5));
	//	btCollisionShape* shape = new btBoxShape(btVector3(0.5, 1, 0.5));
	//	collisionShapes.push_back(shape);

	//	btTransform startTransform;
	//	startTransform.setIdentity();

	//	btScalar mass(1.f);

	//	bool isDynamic = (mass != 0.f);

	//	btVector3 localInertia(0, 0, 0);
	//	if (isDynamic) shape->calculateLocalInertia(mass, localInertia);

	//	for (int i = 0; i < 50; i++)
	//	{
	//		if (i == 0)
	//			startTransform.setOrigin(btVector3(0., 50.0, 0.));
	//		else
	//			startTransform.setOrigin(btVector3(cos((float)i)*5.0f, 150.0f + i*7.0f / sqrt((float)i), sin((float)i)*5.0f));
	//		btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	//		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	//		btRigidBody* body = new btRigidBody(rbInfo);
	//		body->setCenterOfMassTransform(startTransform);
	//		body->setRestitution(0.75);

	//		dynamicsWorld->addRigidBody(body);
	//	}

	//}
}

void Game::createShadowmap()
{
	gl::GenFramebuffers(1, &depthMapFBO);

	gl::GenTextures(1, &depthMap);
	gl::BindTexture(gl::TEXTURE_2D, depthMap);
	gl::TexImage2D(gl::TEXTURE_2D, 0, gl::DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, gl::DEPTH_COMPONENT, gl::FLOAT, NULL);

	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::NEAREST);

	gl::TexParameterf(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_BORDER);
	gl::TexParameterf(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_BORDER);
	float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	gl::TexParameterfv(gl::TEXTURE_2D, gl::TEXTURE_BORDER_COLOR, borderColor);


	gl::BindFramebuffer(gl::FRAMEBUFFER, depthMapFBO);
	gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::TEXTURE_2D, depthMap, 0);
	gl::DrawBuffer(gl::NONE);
	gl::ReadBuffer(gl::NONE);
	gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
}

void Game::Input(float dt)
{
	float speed = 10.f;
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
			if (isWindowFocused) camera.position.y += event.wheel.y * dt * speed;
		}
	}
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LSHIFT]) speed = 3.f;
	if (keys[SDL_SCANCODE_LCTRL]) speed = 20.f;

	if (keys[SDL_SCANCODE_W])
		camera.position += camera.direction * dt * speed;
	if (keys[SDL_SCANCODE_S])
		camera.position -= camera.direction * dt * speed;
	if (keys[SDL_SCANCODE_A])
		camera.position -= glm::cross(glm::vec3(sin(camera.yaw), 0, cos(camera.yaw)), camera.up) * dt * speed;
	if (keys[SDL_SCANCODE_D])
		camera.position += glm::cross(glm::vec3(sin(camera.yaw), 0, cos(camera.yaw)), camera.up) * dt * speed;

	if (keys[SDL_SCANCODE_Q])
		camera.position.y += dt * speed;
	if (keys[SDL_SCANCODE_Z])
		camera.position.y -= dt * speed;

	//if (keys[SDL_SCANCODE_UP]) crane.acceleration = 6.f;
	//else if (keys[SDL_SCANCODE_DOWN]) crane.acceleration = -6.f;
	//else crane.acceleration = 0;

	float leftTrackSpeed = 0;
	float rightTrackSpeed = 0;
	float brake = 0;
	if (keys[SDL_SCANCODE_UP]) leftTrackSpeed = rightTrackSpeed = 100.f;
	if (keys[SDL_SCANCODE_DOWN]) leftTrackSpeed = rightTrackSpeed = -100.f;
	if (keys[SDL_SCANCODE_LEFT]) {
		leftTrackSpeed += 75.f;
		rightTrackSpeed -= 75.f;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		rightTrackSpeed += 75.f;
		leftTrackSpeed -= 75.f;
	}
	if (keys[SDL_SCANCODE_SPACE]) brake = 1.f;


	for (int i = 0; i < crane.vehicle->getNumWheels(); i++)  {
		float craneSpeed = 0;
		if (i < 4) craneSpeed = leftTrackSpeed;
		else craneSpeed = rightTrackSpeed;
		crane.vehicle->applyEngineForce(craneSpeed, i);
		crane.vehicle->setBrake(brake, i);
	}
}


void Game::Step(float dt)
{
	if (!physicsPaused) dynamicsWorld->stepSimulation(dt);
	lightPosition = glm::vec3(20 * cos(lightAngle), 7.0, 20 * sin(lightAngle));
	lightAngle += dt * 0.5f;

	//crane.velocity += crane.acceleration * dt;
	//if (crane.velocity > 15.f) crane.velocity = 15.f;
	//if (crane.velocity < -15.f) crane.velocity = -15.f;
	//crane.velocity *= 0.98f;

	//crane.direction = glm::vec3(
	//	sin(crane.yaw),
	//	0,
	//	cos(crane.yaw));

	//crane.position += crane.direction * crane.velocity * dt;

	crane.timer = glm::length(crane.position)* 5.f;
}

void Game::renderScene(std::string shader)
{
	auto program = resourceManager.getProgram(shader);
	//resourceManager.getTexture("bunny")->use();
	//for (int i = 0; i < dynamicsWorld->getCollisionObjectArray().size() - 1; i++)
	//{
	//	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1 + i];
	//	btRigidBody* body = btRigidBody::upcast(obj);

	//	btTransform transform;
	//	body->getMotionState()->getWorldTransform(transform);

	//	glm::mat4 model = glm::mat4(1.0);
	//	glm::quat rotation(transform.getRotation().w(), transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z());
	//	model = glm::translate(model, glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(), transform.getOrigin().getZ()));
	//	model = model * glm::mat4_cast(rotation);

	//	gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
	//	resourceManager.getModel("bunny")->render();
	//}

	if (shader != shadowmapShader) {
		glm::mat4 model = glm::mat4(1.0);
		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		resourceManager.getTexture("terrain")->use();
		resourceManager.getModel(terrainModel)->render();
		resourceManager.getTexture("skybox")->use();
		resourceManager.getModel("skybox")->render();
	}

	crane.render(shader, viewPhysics);
}

void Game::beginNormalRender()
{
	gl::Viewport(0, 0, resolutionWidth, resolutionHeight);
	gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
	gl::Enable(gl::DEPTH_TEST);
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	gl::Disable(gl::CULL_FACE);
	gl::PolygonMode(gl::FRONT_AND_BACK, wireframe ? gl::LINE : gl::FILL);

	gl::ClearColor(1.f, 1.f, 1.f, 1.0f);
	gl::ClearDepth(1.0f);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

	auto program = resourceManager.getProgram(normalShader);
	program->use();

	camera.direction = glm::vec3(
		cos(camera.pitch) * sin(camera.yaw),
		sin(camera.pitch),
		cos(camera.pitch) * cos(camera.yaw));

	glm::mat4 projection = glm::perspective(fov, (float)resolutionWidth / (float)resolutionHeight, 0.1f, 1000.0f);
	glm::mat4 view = camera.getView();
	gl::UniformMatrix4fv(program->getUniform("proj"), 1, false, glm::value_ptr(projection));
	gl::UniformMatrix4fv(program->getUniform("view"), 1, false, glm::value_ptr(view));

	gl::Uniform3fv(program->getUniform("lightPosition"), 1, glm::value_ptr(lightPosition));
	gl::Uniform3fv(program->getUniform("cameraPosition"), 1, glm::value_ptr(camera.position));
	gl::Uniform3fv(program->getUniform("ambient"), 1, glm::value_ptr(ambient));
	gl::Uniform3fv(program->getUniform("diffuse"), 1, glm::value_ptr(diffuse));
	gl::Uniform3fv(program->getUniform("specular"), 1, glm::value_ptr(specular));
	gl::Uniform1i(program->getUniform("texture"), 0);
	gl::Uniform1i(program->getUniform("shadowMap"), 1);

	gl::ActiveTexture(gl::TEXTURE1);
	gl::BindTexture(gl::TEXTURE_2D, depthMap);
	gl::ActiveTexture(gl::TEXTURE0);

	glm::mat4 lightProjection = glm::ortho(-25.f, 25.f, -25.f, 25.f, 0.1f, 1000.f);
	glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(1.0f));
	glm::mat4 lightSpace = lightProjection * lightView;
	gl::UniformMatrix4fv(program->getUniform("lightSpace"), 1, false, glm::value_ptr(lightSpace));
}

void Game::beginShadowmapRender()
{
	gl::Viewport(0, 0, shadowMapWidth, shadowMapHeight);
	gl::BindFramebuffer(gl::FRAMEBUFFER, depthMapFBO);
	gl::Clear(gl::DEPTH_BUFFER_BIT);
	gl::Enable(gl::DEPTH_TEST);
	gl::Enable(gl::CULL_FACE);
	gl::CullFace(gl::FRONT);

	gl::ActiveTexture(gl::TEXTURE0);
	auto program = resourceManager.getProgram(shadowmapShader);
	program->use();

	glm::mat4 lightProjection = glm::ortho(-25.f, 25.f, -25.f, 25.f, 0.1f, 1000.f);
	glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(1.0f));
	glm::mat4 lightSpace = lightProjection * lightView;
	gl::UniformMatrix4fv(program->getUniform("lightSpace"), 1, false, glm::value_ptr(lightSpace));
}

void Game::Render()
{
	beginShadowmapRender();
	renderScene(shadowmapShader);

	beginNormalRender();
	renderScene(normalShader);

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
	ImGui::Checkbox("View debug", &viewPhysics);
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
	camera.pitch -= ((float)yrel / resolutionHeight) * dt * mouseSpeed;
	camera.pitch = fmax(fmin(camera.pitch, (M_PI / 2) - 0.0001f), (-M_PI / 2) + 0.0001f);

	camera.yaw -= ((float)xrel / resolutionWidth) * dt * mouseSpeed;
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