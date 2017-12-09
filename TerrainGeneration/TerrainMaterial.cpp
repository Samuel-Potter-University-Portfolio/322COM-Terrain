#include "TerrainMaterial.h"

#include "GL\glew.h"


TerrainMaterial::TerrainMaterial()
{
	m_shader = new Shader;
	m_shader->LoadVertexShaderFromFile("Resources\\Shaders\\terrain.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources\\Shaders\\terrain.frag.glsl");
	m_shader->LinkShader();

	m_grassTexture.LoadFromFile("Resources\\grass.png");
	m_dirtTexture.LoadFromFile("Resources\\dirt.png");
	m_sandTexture.LoadFromFile("Resources\\sand.png");
	m_stoneTexture.LoadFromFile("Resources\\stone.png");
}

TerrainMaterial::~TerrainMaterial() 
{
	delete m_shader;
}

void TerrainMaterial::Bind(class Window& window, class Scene& scene) 
{
	SceneMaterial::Bind(window, scene);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_grassTexture.GetID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_dirtTexture.GetID());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_sandTexture.GetID());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_stoneTexture.GetID());
}