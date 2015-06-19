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
	craneMatrix = glm::rotate(craneMatrix, yaw, glm::vec3(0, 1, 0));

	for (auto obj : resourceManager.getModel("crane")->objects)
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
	craneMatrix = glm::rotate(craneMatrix, yaw, glm::vec3(0, 1, 0));

	resourceManager.getTexture(trackTexture)->use();

	for (auto obj : resourceManager.getModel("crane")->objects)
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
void Crane::render(std::string shader)
{
	this->shader = shader;
	renderBody();
	renderTracks();
}
