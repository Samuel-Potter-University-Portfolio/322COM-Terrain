#pragma once
#include "Material.h"
#include "Texture.h"
#include "Shader.h"

#include "PerlinNoise.h"


/**
* The material that is applied when rendering trees
*/
class CloudMaterial : public SceneMaterial
{
private:
	///
	/// Cached uniforms
	///
	uint32 m_uniformClockTime;

	///
	/// Material vars
	///
	Texture m_noiseTexture;

public:
	CloudMaterial(const PerlinNoise* noise);
	virtual ~CloudMaterial();


	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param scene				The scene that will be rendered
	*/
	virtual void Bind(class Window& window, class Scene& scene) override;
};

