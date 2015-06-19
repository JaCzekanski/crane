#pragma once
#include <glm\glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>

class Camera
{
public:
	glm::vec3 position = glm::vec3(0.0f, 5.0f, 15.0f); 
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f); 
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	float pitch = 0.0f;
	float yaw = glm::pi<float>();

	glm::mat4 getView();

};