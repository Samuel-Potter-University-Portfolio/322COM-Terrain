#include "TreeMaterial.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"


TreeMaterial::TreeMaterial()
{
	m_shader = new Shader;
	m_shader->LoadVertexShaderFromFile("Resources\\Shaders\\tree.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources\\Shaders\\tree.frag.glsl");
	m_shader->LinkShader();

	m_barkTexture.LoadFromFile("Resources\\tree_bark.png");
	m_leavesTexture.LoadFromFile("Resources\\leaves.png");

	m_uniformClockTime = m_shader->GetUniform("clockTime");
}

TreeMaterial::~TreeMaterial()
{
	delete m_shader;
}

void TreeMaterial::Bind(class Window& window, class Scene& scene)
{
	SceneMaterial::Bind(window, scene);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_barkTexture.GetID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_leavesTexture.GetID());

	const float time = glfwGetTime();
	m_shader->SetUniformFloat(m_uniformClockTime, time);
}