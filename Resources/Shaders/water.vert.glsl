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


out vec3 passToCamera;
out vec2 passUV0;
out vec2 passUV1;


layout(binding = 0) uniform sampler2D bumpMap;


void main()
{	
	vec4 worldLocation = ObjectToWorld * vec4(inPos, 1);
	passUV0 = (worldLocation.xz + clockTime * windFrequency * windDirection.xz) * 0.01;
	passUV1 = (worldLocation.xz + clockTime * windFrequency * vec2(1,0)) * 0.01;

	
	// Offset vertex by height
	float h = mix(texture(bumpMap, passUV0).b, texture(bumpMap, passUV1).b, 0.5);
	worldLocation.y += h * 0.7;
	gl_Position = ViewToClip * WorldToView * worldLocation;


	vec3 cameraLocation = (inverse(WorldToView) * vec4(0,0,0,1)).xyz;
	passToCamera = worldLocation.xyz - cameraLocation;
}