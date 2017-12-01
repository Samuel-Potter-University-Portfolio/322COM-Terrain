#pragma once
#include "Common.h"
#include "Window.h"

#include "Chunk.h"

#include <thread>


class Scene;


class Terrain
{
private:
	Scene* m_parent;

	///
	/// Threading (Handles chunk generation/management jobs asynchronously)
	///
	std::thread* m_workerThread;
	bool bWorkerRunning = false;

	Chunk* TESTCHUNK;

public:
	Terrain(Scene* scene);
	~Terrain();


	/**
	* Callback for when any logic should happen
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void UpdateScene(Window& window, const float& deltaTime);

	/**
	* Callback when the terrain itself should be renderered
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void RenderTerrain(Window& window, const float& deltaTime);

	//void RenderTrees(Window& window, const float& deltaTime);

	//void RenderWater(Window& window, const float& deltaTime);


private:
	/** Called by the worker thread (Launches into loop) */
	void RunWorker();
};

