#pragma once
#include <string>
#include "../utils/physics.h"

class Crane
{
	void renderBody();
	void renderTracks();
	void renderPhysics();

	float wheelRadius = 0.5f;
	glm::vec3 boxSize = glm::vec3(1.1f, 0.5f, 2.0f);
	std::string shader;
public:
	std::string model = "crane";
	std::string trackTexture = "gasienica";
	glm::vec3 position = glm::vec3(0.0, 3.0, 0.0);
	glm::quat rotation;
	glm::vec3 direction;
	float velocity, acceleration;
	float yaw;
	float timer;
	btRigidBody* body;
	btRigidBody* wheels[8];
	btHinge2Constraint* hinges[8];
	btRaycastVehicle* vehicle;


	void render( std::string shader, bool debug = false );
	void createPhysicsModel( btDiscreteDynamicsWorld *world );
};