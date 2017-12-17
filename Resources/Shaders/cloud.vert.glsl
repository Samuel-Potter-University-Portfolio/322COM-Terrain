#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;
// View matrix
uniform mat4 WorldToView;
// Perspective matrix
uniform mat4 ViewToClip;


uniform float clockTime;
const vec3 windDirection = normalize(vec3(1, 0, 1));
const float windFrequency = 0.5;


layout (location = 0) in vec3 inPos;

out vec2 passUVs;


void main()
{
	// Don't translate at all
	vec3 cameraLocation = (inverse(WorldToView) * vec4(0,0,0,1)).xyz;
	vec4 worldLocation = ObjectToWorld * vec4(inPos, 1);
	worldLocation.xz += cameraLocation.xz;

	gl_Position = ViewToClip * vec4((WorldToView * worldLocation).xyz, 1);
	passUVs = (worldLocation.xz + cameraLocation.xz) * 0.001 + (clockTime * windFrequency * windDirection.xz + worldLocation.y * 0.1) * 0.01;
}