#pragma once
#include "Common.h"

#include "Window.h"
#include "Camera.h"

#include "Terrain.h"
#include "SkyMaterial.h"
#include "CloudMaterial.h"


/**
* Contains and manages all of the states for displaying
* the terrain and objects
*/
class Scene
{
private:
	///
	/// Controllers
	///
	Camera m_camera;
	float m_cameraCurrentSpeed;
	Terrain* m_terrain = nullptr;

	///
	/// Skybox
	///
	Mesh*			m_skyMesh;
	Mesh*			m_cloudMesh;
	SkyMaterial*	m_skyMaterial;
	CloudMaterial*	m_cloudMaterial;


public:
	/** Loads all required assets and systems needed by the scene */
	void Build();
	/** Destroys all assets and systems used by the scene */
	void Destroy();

	/**
	* Callback by the window when this scene is ready to update/render
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	inline void UpdateCallback(Window& window, const float& deltaTime) 
	{
		UpdateScene(window, deltaTime);
		RenderScene(window, deltaTime);
	}

private:

	/**
	* Callback for when any logic should happen
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void UpdateScene(Window& window, const float& deltaTime);
	/**
	* Callback by when the scene should be renderered
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void RenderScene(Window& window, const float& deltaTime);


	///
	/// Getters & Setters
	///
public:
	inline Camera& GetCamera() { return m_camera; }
};

