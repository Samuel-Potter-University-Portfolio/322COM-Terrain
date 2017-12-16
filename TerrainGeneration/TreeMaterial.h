#pragma once
#include "Material.h"
#include "Texture.h"
#include "Shader.h"


/**
* The material that is applied when rendering trees
*/
class TreeMaterial : public SceneMaterial
{
private:
	///
	/// Material vars
	///
	Texture m_barkTexture;
	Texture m_leavesTexture;

public:
	TreeMaterial();
	virtual ~TreeMaterial();


	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param scene				The scene that will be rendered
	*/
	virtual void Bind(class Window& window, class Scene& scene) override;
};

