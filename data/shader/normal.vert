#version 150

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lightPosition;

out vec3 fragNormal;
out vec3 fragPosition;

void main()
{
	fragNormal = normal;
	fragPosition = position;

	gl_Position = proj * view * model * vec4(position, 1.0);
}