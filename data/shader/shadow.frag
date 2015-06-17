#version 150

in vec4 shadowCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform vec3 diffuse = vec3( 0.75, 0.75, 0.75 );
uniform vec3 ambient = vec3( 0.75, 0.75, 0.75 );
uniform vec3 specular = vec3( 0.0, 0.0, 0.0 );

uniform sampler2D shadowMap;

uniform sampler2D texture;


void main()
{
	vec4 shadowCoordinateWdivide = shadowCoord / shadowCoord.w;

	shadowCoordinateWdivide.z += 0.0005;

	float distanceFromLight = texture2D(shadowMap, shadowCoordinateWdivide.st).z;

	float shadow = 1.0;
	if (shadowCoord.w > 0.0)
		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0;

	gl_FragColor = shadow;
}