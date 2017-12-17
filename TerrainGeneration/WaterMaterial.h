#pragma once
#include "Material.h"
#include "Texture.h"
#include "Shader.h"


/**
* The material that is applied when rendering water
*/
class WaterMaterial : public SceneMaterial
{
private:
	///
	/// Cached uniforms
	///
	uint32 m_uniformClockTime;

	///
	/// Material vars
	///
	Texture m_bumpTexture;
	Texture m_normalTexture;

public:
	WaterMaterial();
	virtual ~WaterMaterial();


	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param scene				The scene that will be rendered
	*/
	virtual void Bind(class Window& window, class Scene& scene) override;
};

