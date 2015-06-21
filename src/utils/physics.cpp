#include "physics.h"

btRigidBody* createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape)
{
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(info);
	//body->setCenterOfMassTransform(startTransform);

	//body->setUserIndex(-1);
	//m_dynamicsWorld->addRigidBody(body);
	return body;
}

glm::vec3 convert(const btVector3 &i)
{
	glm::vec3 o;
	o.x = i.x();
	o.y = i.y();
	o.z = i.z();
	return o;
}

btVector3 convert(const glm::vec3 &i)
{
	btVector3 o;
	o.setX(i.x);
	o.setY(i.y);
	o.setZ(i.z);
	return o;
}

glm::quat convert(const btQuaternion &i)
{
	glm::quat o;
	o.x = i.x();
	o.y = i.y();
	o.z = i.z();
	o.w = i.w();
	return o;
}