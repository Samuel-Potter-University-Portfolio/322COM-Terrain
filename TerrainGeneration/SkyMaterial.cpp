#include "SkyMaterial.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"


SkyMaterial::SkyMaterial()
{
	m_shader = new Shader;
	m_shader->LoadVertexShaderFromFile("Resources\\Shaders\\sky.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources\\Shaders\\sky.frag.glsl");
	m_shader->LinkShader();


	m_skyTexture.SetSmooth(true);
	m_skyTexture.LoadCubemapFromFiles(
		"Resources\\Sky\\Back.png",
		"Resources\\Sky\\Front.png",
		"Resources\\Sky\\Left.png",
		"Resources\\Sky\\Right.png",
		"Resources\\Sky\\Up.png",
		"Resources\\Sky\\Down.png"
		);

	m_uniformClockTime = m_shader->GetUniform("clockTime");
}

SkyMaterial::~SkyMaterial()
{
	delete m_shader;
}

void SkyMaterial::Bind(class Window& window, class Scene& scene)
{
	SceneMaterial::Bind(window, scene);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexture.GetID());

	const float time = glfwGetTime();
	m_shader->SetUniformFloat(m_uniformClockTime, time);
}