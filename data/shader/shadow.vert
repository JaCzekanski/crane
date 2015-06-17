#version 330

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lightPosition;

out vec4 shadowCoord;

void main()
{
	shadowCoord = proj * view * model * vec4(position, 1.0);
	gl_Position = ftransform();//proj * view * model * vec4(position, 1.0);
	gl_FrontColor = gl_Color;
}