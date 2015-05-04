#version 150

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform sampler2D texture;

const vec3 diffuseColor = vec3( 0.5, 0.5, 0.5 );
const vec3 diffuse_Color = vec3( 0.2, 0.2, 0.2 );
const vec3 ambientColor = vec3( 0.3, 0.3, 0.3 );

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 normal = normalize( normalMatrix * fragNormal );

	vec3 fragPosition = vec3( model * vec4(fragPosition, 1));

	vec3 surfaceToLight = lightPosition - fragPosition;
	float brightness =  dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
	brightness = clamp(brightness, 0, 1);

	vec3 surfaceToLight_ = cameraPosition - fragPosition;
	float brightness_ = dot(normal, surfaceToLight_) / (length(surfaceToLight_) * length(normal));
	brightness_ = clamp(brightness_, 0, 1);


	vec3 color = brightness * diffuseColor + brightness_ * diffuse_Color + ambientColor;

	gl_FragColor =  texture2D(tex, fragCoords) * vec4( color, 1.0 );
}