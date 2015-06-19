#include "Camera.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\euler_angles.hpp>
#include "../resource/ResourceManager.h"

glm::mat4 Camera::getView()
{
	return glm::lookAt(
		position,
		position + direction,
		up);
}