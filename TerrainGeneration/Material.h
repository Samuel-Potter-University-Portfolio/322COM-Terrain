#pragma once
#include "Common.h"
#include "Shader.h"



/**
* Creates an instance of a shader and connect any required processing
* e.g. Storing/Loading uniforms or textures
*/
class Material
{
protected:
	Shader* m_shader = nullptr;

public:
	Material() {};
	virtual ~Material() {};


	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param scene				The scene that will be rendered
	*/
	virtual void Bind(class Window& window, class Scene& scene) = 0;

	/**
	* Prepares this mesh for rendering
	* @param mesh				The mesh that is going to be rendered
	* @param transform			The transform to use to render this mesh with
	*/
	virtual void PrepareMesh(class Mesh& mesh) = 0;

	/**
	* Render an instance of the previously bound mesh at this transform
	* @param transform			The transform data to use during render
	*/
	virtual void RenderInstance(class Transform& transform) = 0;
};



/**
* Default scene material implementation that will bind the shader and load default scene uniforms
*/
class SceneMaterial : public Material 
{
protected:
	///
	/// Cached uniforms
	///
	uint32 m_uniformObjectToWorld;
	uint32 m_uniformWorldToView;
	uint32 m_uniformViewToClip;

	uint32 m_boundMeshTris;


public:
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
	virtual void PrepareMesh(class Mesh& mesh);

	/**
	* Render an instance of the previously bound mesh at this transform
	* @param transform			The transform data to use during render
	*/
	virtual void RenderInstance(class Transform& transform);
};
