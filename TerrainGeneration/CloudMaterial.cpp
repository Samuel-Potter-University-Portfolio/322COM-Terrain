#include "CloudMaterial.h"
#include "GL\glew.h"
#include "GLFW\glfw3.h"


CloudMaterial::CloudMaterial(const PerlinNoise* noise)
{
	m_shader = new Shader;
	m_shader->LoadVertexShaderFromFile("Resources\\Shaders\\cloud.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources\\Shaders\\cloud.frag.glsl");
	m_shader->LinkShader();


	// Create a noise map that can be used for clouds
	const float cloudScale = 0.006f;
	const uint32 width = 1024;
	std::vector<uint8> texData;
	texData.resize(width * width);

	for (uint32 x = 0; x < width; ++x)
		for (uint32 y = 0; y < width; ++y)
			texData[y * width + x] = (uint8)(noise->GetOctave(x * cloudScale, y * cloudScale, 2, 0.8) * 255);
	m_noiseTexture.LoadFromMemory(texData.data(), width, width, GL_RED);

	// Force the repeat modes
	glBindTexture(GL_TEXTURE_2D, m_noiseTexture.GetID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);


	m_uniformClockTime = m_shader->GetUniform("clockTime");
}

CloudMaterial::~CloudMaterial()
{
	delete m_shader;
}

void CloudMaterial::Bind(class Window& window, class Scene& scene)
{
	SceneMaterial::Bind(window, scene);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_noiseTexture.GetID());

	const float time = glfwGetTime();
	m_shader->SetUniformFloat(m_uniformClockTime, time);
}