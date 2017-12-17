#include "WaterMaterial.h"
#include "Scene.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"


WaterMaterial::WaterMaterial()
{
	m_shader = new Shader;
	m_shader->LoadVertexShaderFromFile("Resources\\Shaders\\water.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources\\Shaders\\water.frag.glsl");
	m_shader->LinkShader();

	m_bumpTexture.SetSmooth(true);
	m_bumpTexture.LoadFromFile("Resources\\water_bump.png");
	m_normalTexture.SetSmooth(true);
	m_normalTexture.LoadFromFile("Resources\\water_normal.png");

	m_uniformClockTime = m_shader->GetUniform("clockTime");
}

WaterMaterial::~WaterMaterial()
{
	delete m_shader;
}

void WaterMaterial::Bind(class Window& window, class Scene& scene)
{
	SceneMaterial::Bind(window, scene);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_bumpTexture.GetID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture.GetID());

	const float time = glfwGetTime();
	m_shader->SetUniformFloat(m_uniformClockTime, time);
}