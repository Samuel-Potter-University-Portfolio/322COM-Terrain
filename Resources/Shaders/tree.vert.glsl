#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;
// View matrix
uniform mat4 WorldToView;
// Perspective matrix
uniform mat4 ViewToClip;


uniform float clockTime;
const vec3 windDirection = normalize(vec3(1, 0, 1));
const float windForce = 0.7;
const float windFrequency = 0.5;


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUVs;
layout (location = 4) in vec2 inExtraData;

out vec3 passPos;
out vec3 passNormal;
out vec2 passUVs;

out flat int textureType;


void main()
{
	// Make vertex sway in wind
	vec3 sway;
	float swayWeight = inExtraData.y;
	sway = windDirection * windForce * swayWeight * sin((clockTime + mod(max((100 - dot(inPos, inPos)) * 0.1, 0), 10)) * windFrequency);


	vec4 worldLocation = ObjectToWorld * vec4(inPos + sway, 1);
	gl_Position = ViewToClip * WorldToView * worldLocation;

	passPos = worldLocation.xyz;
	passNormal = inNormal;
	passUVs = inUVs;
	
	textureType = int(inExtraData.x);
}