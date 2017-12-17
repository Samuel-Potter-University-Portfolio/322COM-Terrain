#version 420 core
		

in vec3 passToCamera;
in vec3 passPos;
in vec3 passNormal;
in vec4 passColour;


out vec4 outColour;


layout(binding = 0) uniform sampler2D texChannel0;
layout(binding = 1) uniform sampler2D texChannel1;
layout(binding = 2) uniform sampler2D texChannel2;
layout(binding = 3) uniform sampler2D texChannel3;


/// Get the texture colour for this normal
vec4 GetTextureColour(sampler2D tex, vec3 anorm)
{
	vec4 value;
	vec3 blend = anorm;
	blend /= blend.x + blend.y + blend.z;
			
	// Perform tri-planar projection
	if(blend.x != 0)
		value = blend.x * texture(tex, passPos.yz);
	if(blend.y != 0)
		value = value + blend.y * texture(tex, passPos.xz);
	if(blend.z != 0)
		value = value + blend.z * texture(tex, passPos.xy);

	return value;
}


/// Calculate colour
vec4 GetColour()
{
	vec3 anorm = normalize(vec3(abs(passNormal.x), abs(passNormal.y), abs(passNormal.z)));
	vec4 textureWeights = normalize(passColour);
	textureWeights /= textureWeights.x + textureWeights.y + textureWeights.z + textureWeights.w;
	vec4 value;


	if(textureWeights.r != 0)
		value = textureWeights.r * GetTextureColour(texChannel0, anorm);
	if(textureWeights.g != 0)
		value = value + textureWeights.g * GetTextureColour(texChannel1, anorm);
	if(textureWeights.b != 0)
		value = value + textureWeights.b * GetTextureColour(texChannel2, anorm);
	if(textureWeights.a != 0)
		value = value + textureWeights.a * GetTextureColour(texChannel3, anorm);

	return value;
}


void main()
{
	vec3 lightDirection = normalize(vec3(1, -1, 1));
	vec3 normal = normalize(passNormal);
	float diffuse = max(dot(-lightDirection, normal), 0.2);

	// Test texture based on normal/face
	//outColour.rgb = texture(texChannel1, GetUVs()).rgb;
	outColour = GetColour() * diffuse;
	outColour.a = 1.0;
}