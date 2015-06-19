#pragma once
#include <glm\glm.hpp>
#include <string>

class Crane
{
	void renderBody();
	void renderTracks();

	std::string shader;
public:
	std::string model = "crane";
	std::string trackTexture = "gasienica";
	glm::vec3 position = glm::vec3(0.0, 3.0, 0.0);
	glm::vec3 direction;
	float velocity, acceleration;
	float yaw;
	float timer;


	void render( std::string shader );
};