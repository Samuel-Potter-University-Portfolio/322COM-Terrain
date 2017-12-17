#version 420 core

in vec3 passToCamera;
in vec2 passUV0;
in vec2 passUV1;


out vec4 outColour;


layout(binding = 1) uniform sampler2D normalMap;


void main()
{
	vec3 lightDirection = normalize(vec3(1, -1, 1));
	vec3 normal = normalize(mix(texture(normalMap, passUV0).xzy, texture(normalMap, passUV1).xzy, 0.5) * 2.0 - 1.0);

	float diffuse = max(dot(-lightDirection, normal), 0.2);

	vec3 toCamera = normalize(passToCamera);
	vec3 reflectedLight = reflect(-lightDirection, normal);

	float specular = max(0.0, dot(reflectedLight, toCamera));


	outColour.rgb = vec3(0.251, 0.643, 0.875) * diffuse + pow(specular, 1.0);
	outColour.a = 0.3 + specular * 0.5;
}