#pragma once
#include "Common.h"
#include "Shader.h"


/**
* Creates an instance of a shader and connect any required processing
* e.g. Storing/Loading uniforms or textures
*/
class Material
{
private:
	Shader* m_shader;

protected:
	///
	/// Cached uniforms
	///
	uint32 m_uniformObjectToWorld;
	uint32 m_uniformWorldToView;
	uint32 m_uniformViewToClip;

public:
	Material(Shader* shader);
	~Material();


	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param scene				The scene that will be rendered
	*/
	virtual void Bind(class Window& window, class Scene& scene);

	/**
	* Prepares this mesh for rendering
	* @param mesh				The mesh that is going to be rendered
	* @param transform			The transform to use to render this mesh with
	*/
	virtual void PrepareMesh(class Mesh& mesh, class Transform& transform);
};

