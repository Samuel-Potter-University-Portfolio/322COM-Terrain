#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;

// View matrix
uniform mat4 WorldToView;

// Perspective matrix
uniform mat4 ViewToClip;


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUVs;
layout (location = 4) in vec2 inExtraData;

out vec3 passPos;
out vec3 passNormal;
out vec2 passUVs;

out flat int textureType;
out float swayWeight;


void main()
{
	vec4 worldLocation = ObjectToWorld * vec4(inPos, 1);
	gl_Position = ViewToClip * WorldToView * worldLocation;

	passPos = worldLocation.xyz;
	passNormal = inNormal;
	passUVs = inUVs;
	
	textureType = int(inExtraData.x);
	swayWeight = inExtraData.y;
}