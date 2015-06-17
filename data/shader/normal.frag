#version 150

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

// Material
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

uniform sampler2D texture;


void main()
{
	vec3 P = vec3( model * vec4(fragPosition, 1));

	vec3 N = normalize( transpose(inverse(mat3(model))) * fragNormal );
	vec3 V = normalize( cameraPosition - P );
	vec3 L = normalize( lightPosition - P );

	float df =     max(0.0, dot(N, L));  // Diffuse 
	float sf = pow(max(0.0, dot(V, reflect(-L, N))), 32); // Specular

	vec3 light = ambient + df * diffuse + sf * specular;
	gl_FragColor =  texture2D(texture, fragCoord) * vec4( light, 1.0 );
}