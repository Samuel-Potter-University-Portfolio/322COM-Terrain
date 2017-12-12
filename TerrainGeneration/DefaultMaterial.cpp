#include "DefaultMaterial.h"



DefaultMaterial::DefaultMaterial()
{
	m_shader = new Shader;
	// Just transform object into position
	m_shader->LoadVertexShaderFromMemory(R"(
		#version 420 core

		// Transform matrix
		uniform mat4 ObjectToWorld;

		// View matrix
		uniform mat4 WorldToView;

		// Perspective matrix
		uniform mat4 ViewToClip;


		layout (location = 0) in vec3 inPos;

		out vec3 passPos;


		void main()
		{
			vec4 worldLocation = ObjectToWorld * vec4(inPos, 1);
			gl_Position = ViewToClip * WorldToView * worldLocation;
			passPos = worldLocation.xyz;
		}
	)"); 
	// Shade object by coordinates
	m_shader->LoadFragmentShaderFromMemory(R"(
		#version 420 core
		

		in vec3 passPos;

		out vec4 outColour;


		void main()
		{
			outColour.r = mod(passPos.r, 1.0);
			outColour.g = mod(passPos.g, 1.0);
			outColour.b = mod(passPos.b, 1.0);
			outColour.a = 1.0;
		}
	)");
	m_shader->LinkShader();
}


DefaultMaterial::~DefaultMaterial()
{
	delete m_shader;
}
