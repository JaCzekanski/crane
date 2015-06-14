#version 150

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform vec3 diffuse = vec3( 0.75, 0.75, 0.75 );
uniform vec3 ambient = vec3( 0.75, 0.75, 0.75 );
uniform vec3 specular = vec3( 0.0, 0.0, 0.0 );

uniform sampler2D texture;


void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 normal = normalize( normalMatrix * fragNormal );

	vec3 fragPosition = vec3( model * vec4(fragPosition, 1));

	vec3 surfaceToLight = lightPosition - fragPosition;
	float brightness =  dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
	brightness = clamp(brightness, 0, 1);

	vec3 color = brightness * diffuse + ambient;

	gl_FragColor =  texture2D(texture, fragCoord) * vec4( color, 1.0 );
}