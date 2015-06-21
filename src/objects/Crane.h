#pragma once
#include <string>
#include "../utils/physics.h"

class Crane
{
	void renderBody();
	void renderTracks();
	void renderPhysics();

	float wheelRadius = 0.5f;
	glm::vec3 baseSize = glm::vec3(1.1f, 0.8f, 2.0f);
	glm::vec3 cabinSize = glm::vec3(2.0f, 1.0f, 3.5f);
	glm::vec3 armSize = glm::vec3(0.4f, 0.4f, 5.0f);
	float ballRadius = 0.5f;
	std::string shader;
public:
	std::string model = "crane";
	std::string trackTexture = "gasienica";
	btRigidBody* base;
	btRigidBody* cabin;
	btRigidBody* arm;
	btRigidBody* ball;
	btRaycastVehicle* vehicle;

	void render( std::string shader, bool debug = false );
	void createPhysicsModel( btDiscreteDynamicsWorld *world );
};