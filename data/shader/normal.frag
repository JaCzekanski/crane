#version 330 

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragCoord;
in vec4 fragPositionLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 lightSpace;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

// Material
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

uniform sampler2D texture;
uniform sampler2D shadowMap;

vec3 N;
vec3 V;
vec3 L;

float ShadowCalculation(vec4 pos) //lightSpace
{
	vec3 projCoords = pos.xyz / pos.w; // pos.w ???
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture2D(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.005 * (1.0 - dot(N, L)), 0.00005);
	//float bias = 0.00005;

	float shadow = 0; 

	// PCF
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for (int y = -1; y <= 1; ++y)
	for (int x = -1; x <= 1; ++x)
	{
		float pcfDepth = texture2D(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}

	if (projCoords.z > 1.0) shadow = 0.0;

	return shadow;
}

void main()
{
	vec3 P = vec3( model * vec4(fragPosition, 1));

	N = normalize( transpose(inverse(mat3(model))) * fragNormal );
	V = normalize( cameraPosition - P );
	L = normalize( lightPosition - P );

	float df =     max(0.0, dot(N, L));  // Diffuse 
	float sf = pow(max(0.0, dot(V, reflect(-L, N))), 32); // Specular

	//Shadow
	float shadow = ShadowCalculation( fragPositionLightSpace );

	vec3 light = (1.0 - shadow*0.08) * ( ambient + df * diffuse + sf * specular) ;

	gl_FragColor =  texture2D(texture, fragCoord) * vec4( light, 1.0 );
}