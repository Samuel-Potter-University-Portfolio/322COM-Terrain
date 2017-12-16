#include "TreeMaterial.h"

#include "GL\glew.h"


TreeMaterial::TreeMaterial()
{
	m_shader = new Shader;
	m_shader->LoadVertexShaderFromFile("Resources\\Shaders\\tree.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources\\Shaders\\tree.frag.glsl");
	m_shader->LinkShader();

	m_barkTexture.LoadFromFile("Resources\\tree_bark.png");
	m_leavesTexture.LoadFromFile("Resources\\leaves.png");
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
}