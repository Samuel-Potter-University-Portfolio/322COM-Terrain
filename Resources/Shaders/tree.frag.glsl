#version 420 core

in vec3 passPos;
in vec3 passNormal;
in vec2 passUVs;

in flat int textureType;


out vec4 outColour;


layout(binding = 0) uniform sampler2D texBark;
layout(binding = 1) uniform sampler2D texLeaves;


void main()
{
	vec3 lightDirection = normalize(vec3(1, -1, 1));
	vec3 normal = normalize(passNormal);
	float diffuse = max(dot(-lightDirection, normal), 0.2);

	vec4 texColour;
	if(textureType == 0)
		texColour = texture(texBark, passUVs * 3);
	else
	{
		texColour = texture(texLeaves, passUVs);
		if(texColour.a < 0.5)
			discard;
	}

	outColour.rgb = texColour.rgb * diffuse;
	outColour.a = 1.0;
}