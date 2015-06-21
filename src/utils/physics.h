#pragma once
#include <btBulletDynamicsCommon.h>
#include <glm\glm.hpp>
#include <glm\gtx\euler_angles.hpp>

btRigidBody* createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape);

glm::vec3 convert(const btVector3 &i);
btVector3 convert(const glm::vec3 &i);
glm::quat convert(const btQuaternion &i);