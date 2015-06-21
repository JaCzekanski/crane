#include "Crane.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\euler_angles.hpp>
#include "../resource/ResourceManager.h"

void Crane::renderBody()
{
	auto program = resourceManager.getProgram(shader);

	glm::mat4 craneMatrix = glm::mat4(1.0);
	craneMatrix = glm::translate(craneMatrix, position);
	craneMatrix = craneMatrix * glm::mat4_cast(rotation);
	//craneMatrix = glm::rotate(craneMatrix, yaw, glm::vec3(0, 1, 0));

	for (auto obj : resourceManager.getModel(model)->objects)
	{
		glm::mat4 model = craneMatrix;

		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		if (!obj.second->material.texture.empty())
			resourceManager.getTexture(obj.second->material.texture)->use();

		gl::Uniform3fv(program->getUniform("diffuse"), 1, glm::value_ptr(obj.second->material.diffuse));
		gl::Uniform3fv(program->getUniform("ambient"), 1, glm::value_ptr(obj.second->material.ambient));
		gl::Uniform3fv(program->getUniform("specular"), 1, glm::value_ptr(obj.second->material.specular));

		obj.second->render();
	}
}
void Crane::renderTracks()
{
	auto program = resourceManager.getProgram(shader);

	glm::mat4 craneMatrix = glm::mat4(1.0);
	craneMatrix = glm::translate(craneMatrix, position);
	craneMatrix = craneMatrix * glm::mat4_cast(rotation);
	//craneMatrix = glm::rotate(craneMatrix, yaw, glm::vec3(0, 1, 0));

	resourceManager.getTexture(trackTexture)->use();

	for (auto obj : resourceManager.getModel(model)->objects)
	{
		if (obj.first != "GosienicePraweSzlak_BezierCircle.001" &&
			obj.first != "GasieniceLeweSzlak_BezierCircle") continue;

		for (auto segment : obj.second->segments)
		{
			glm::mat4 model = craneMatrix;
			model = glm::translate(craneMatrix, segment.getPosition(1.f - timer));
			model = glm::rotate(model, segment.getAngle() - glm::radians(90.f), glm::vec3(1, 0, 0));

			gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
			resourceManager.getModel("gasieniacapart")->render();
		}
	}
}

void Crane::renderPhysics()
{
	auto program = resourceManager.getProgram(shader);

	GLint polygonMode;
	gl::GetIntegerv(gl::POLYGON_MODE, &polygonMode);
	gl::PolygonMode(gl::FRONT_AND_BACK, gl::LINE);

	// Base
	{
		auto transform = base->getWorldTransform();
		glm::mat4 rot = glm::mat4_cast(convert(transform.getRotation()));
		glm::mat4 scale = glm::scale(glm::mat4(1.f), baseSize);
		glm::mat4 translate = glm::translate(glm::mat4(1.f), convert(transform.getOrigin()));
		glm::mat4 model = translate * rot *scale;

		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		resourceManager.getTexture("oslona")->use();

		resourceManager.getModel("cube")->render();
	}

	// Cabin
	{
		auto transform = cabin->getWorldTransform();
		glm::mat4 rot = glm::mat4_cast(convert(transform.getRotation()));
		glm::mat4 scale = glm::scale(glm::mat4(1.f), cabinSize);
		glm::mat4 translate = glm::translate(glm::mat4(1.f), convert(transform.getOrigin()));
		glm::mat4 model = translate * rot *scale;

		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		resourceManager.getTexture("oslona")->use();

		resourceManager.getModel("cube")->render();
	}

	// Cabin
	{
		auto transform = arm->getWorldTransform();
		glm::mat4 rot = glm::mat4_cast(convert(transform.getRotation()));
		glm::mat4 scale = glm::scale(glm::mat4(1.f), armSize);
		glm::mat4 translate = glm::translate(glm::mat4(1.f), convert(transform.getOrigin()));
		glm::mat4 model = translate * rot *scale;

		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		resourceManager.getTexture("oslona")->use();

		resourceManager.getModel("cube")->render();
	}


	// Wheels
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		btTransform tr = vehicle->getWheelInfo(i).m_worldTransform;
		glm::mat4 rot = glm::mat4_cast(convert(tr.getRotation()));
		glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(wheelRadius*0.75, wheelRadius, wheelRadius));
		glm::mat4 translate = glm::translate(glm::mat4(1.f), convert(tr.getOrigin()));
		glm::mat4 model = translate * rot * scale;
		gl::UniformMatrix4fv(program->getUniform("model"), 1, false, glm::value_ptr(model));
		
		resourceManager.getModel("cube")->render();
	}


	gl::PolygonMode(gl::FRONT_AND_BACK, polygonMode);
}

void Crane::render(std::string shader, bool debug)
{
	this->shader = shader;

	position = convert(base->getWorldTransform().getOrigin());
	rotation = convert(base->getWorldTransform().getRotation());

	if (debug) renderPhysics();

	position += glm::vec3(0.0, -0.4, 0.0);

	renderBody();
	renderTracks();
}


void Crane::createPhysicsModel(btDiscreteDynamicsWorld *world)
{
	btVector3 startPosition(0.0, 1.0, 0.0);
	// Body
	btCollisionShape* bodyBase = new btBoxShape(btVector3(baseSize.x*0.5, baseSize.y*0.5, baseSize.z*0.5));

	btTransform tr;

	tr.setIdentity();
	tr.setOrigin(startPosition);

	base = createRigidBody(1000.f, tr, bodyBase);
	base->setActivationState(DISABLE_DEACTIVATION);
	world->addRigidBody(base);

	{
		btCollisionShape* bodyCabin = new btBoxShape(btVector3(cabinSize.x*0.5, cabinSize.y*0.5, cabinSize.z*0.5));

		tr.setIdentity();
		tr.setOrigin(startPosition + btVector3(0, cabinSize.y*0.5 + baseSize.y*0.5, 0));

		cabin = createRigidBody(100.f, tr, bodyCabin);
		cabin->setDamping(0, 0.8);
		world->addRigidBody(cabin);

		btTransform tr1;
		tr1.setIdentity();
		tr1.setOrigin(btVector3(0, cabinSize.y*0.5 + baseSize.y*0.5, 0));

		btFixedConstraint* hinge = new btFixedConstraint(*base, *cabin, tr, tr1);
		//btHinge2Constraint* hinge = new btHinge2Constraint(*base, *cabin, tr.getOrigin(), btVector3(0, 1, 0), btVector3(1, 0, 1));
		//btGeneric6DofConstraint* hinge = new btGeneric6DofConstraint(*base, *cabin, tr1, tr2, true);
		//hinge->setLinearLowerLimit(btVector3(0,0,0));
		//hinge->setLinearUpperLimit(btVector3(0, 0, 0));
		//cabin->setFriction(50);
		world->addConstraint(hinge, true);
	}
	{
		btCollisionShape* bodyArm = new btBoxShape(btVector3(armSize.x*0.5, armSize.y*0.5, armSize.z*0.5));

		tr.setIdentity();
		tr.setRotation(btQuaternion(btVector3(1, 0, 0), glm::radians(-32.f)));
		tr.setOrigin(startPosition + btVector3(0, cabinSize.y*0.5 + baseSize.y*0.5 + armSize.z * 0.35, armSize.z - 1.0));

		arm = createRigidBody(10.f, tr, bodyArm);
		world->addRigidBody(arm);

		tr.setOrigin(tr.getOrigin() - 2.0*startPosition);
		btTransform tr_;
		tr_.setIdentity();

		btFixedConstraint* hinge = new btFixedConstraint(*cabin, *arm, tr, tr_);
		world->addConstraint(hinge, true);
	}

	btRaycastVehicle::btVehicleTuning tuning;
	btVehicleRaycaster* vehicleRayCaster = new btDefaultVehicleRaycaster(world);
	vehicle = new btRaycastVehicle(tuning, base, vehicleRayCaster);

	world->addVehicle(vehicle);

	btVector3 wheelDirection(0, -1, 0);
	btVector3 wheelAxle(-1, 0, 0);
	btScalar suspensionRestLength(0.6);

	vehicle->setCoordinateSystem(0, 1, 2);

	for (int i = 0; i < 8; i++)
	{
		btVector3 connectionPoint = btVector3(
			i<4 ? -1.0 : 1.0, 
			0.5, 
			(-2.4*wheelRadius) + ((i % 4)*(wheelRadius*1.6)));

		vehicle->addWheel(connectionPoint,
				wheelDirection,
				wheelAxle,
				suspensionRestLength,
				wheelRadius, tuning, false);

		btWheelInfo& wheel = vehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = 25.f;
		wheel.m_wheelsDampingRelaxation = 1.5f;
		wheel.m_wheelsDampingCompression = 1.5f;
		wheel.m_maxSuspensionTravelCm = 10;
		wheel.m_frictionSlip = 10;
		wheel.m_rollInfluence = 0.0f;
	}
}