#version 420 core

in vec2 passUVs;


out vec4 outColour;


layout(binding = 0) uniform sampler2D texSky;



void main()
{
	float cloudLevel = texture(texSky, passUVs).r;

	if(cloudLevel < 0.5)
		discard;

	outColour = vec4(1,1,1, min((cloudLevel - 0.5) * 4, 1));
}