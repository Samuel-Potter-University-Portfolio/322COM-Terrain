#pragma once
#include "Material.h"
#include "Texture.h"
#include "Shader.h"


/**
* The material that is applied when rendering terrain
*/
class TerrainMaterial : public SceneMaterial
{
private:
	///
	/// Material vars
	///
	Texture m_grassTexture;
	Texture m_dirtTexture;
	Texture m_sandTexture;
	Texture m_stoneTexture;

public:
	TerrainMaterial();
	virtual ~TerrainMaterial();

	
	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param scene				The scene that will be rendered
	*/
	virtual void Bind(class Window& window, class Scene& scene) override;
};

