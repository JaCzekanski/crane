#version 330

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 lightSpace;

void main()
{
	gl_Position = lightSpace * model * vec4(position, 1.0);
}