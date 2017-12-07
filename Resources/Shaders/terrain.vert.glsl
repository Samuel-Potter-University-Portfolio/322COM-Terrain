#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;

// View matrix
uniform mat4 WorldToView;

// Perspective matrix
uniform mat4 ViewToClip;


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColour;


out vec3 passToCamera;
out vec3 passPos;
out vec3 passNormal;
out vec4 passColour;


void main()
{
	vec4 worldLocation = ObjectToWorld * vec4(inPos, 1);
	vec3 cameraLocation = vec3(ViewToClip[0][3], ViewToClip[1][3], ViewToClip[2][3]);
	passToCamera = worldLocation.xyz - cameraLocation;

	gl_Position = ViewToClip * WorldToView * worldLocation;
	passPos = worldLocation.xyz;
	passNormal = inNormal;
	passColour = inColour;
}