#pragma once
#include "Common.h"
#include "Window.h"

#include "Chunk.h"

#include <thread>

#include <unordered_map>
#include <vector>
#include <queue>


class Scene;


/**
* The appropriate hashing functions which are needed to use ivec2 as a key
* https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
*/
struct ivec2_KeyFuncs
{
	inline size_t operator()(const ivec2& v)const
	{
		return std::hash<int>()(v.x) ^ std::hash<int>()(v.y);
	}

	inline bool operator()(const ivec2& a, const ivec2& b)const
	{
		return a.x == b.x && a.y == b.y;
	}
};



/**
* Holds and synchronises terrain generation and other jobs
*/
class Terrain
{
private:
	Scene* m_parent;

	///
	/// Threading (Handles chunk generation/management jobs asynchronously)
	///
	std::thread* m_workerThread;
	bool bWorkerRunning = false;

	///
	/// Chunk pooling
	///
	const uint32 m_poolSize = 40;
	std::unordered_map<ivec2, Chunk*, ivec2_KeyFuncs> m_activeChunks;
	std::queue<Chunk*> m_chunkPool;


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
	/**
	* Attempt to get a new chunk
	* @param outChunk		Where the chunk will be store, if successful
	* @param coord			The coordinates this chunk should be at
	* @returns If successfully retreived a new chunk
	*/
	bool TryGetNewChunk(Chunk*& outChunk, const ivec2& coord);
	/**
	* Adds a chunk back into the pool
	* @param chunk			The chunk to add back
	*/
	void FreeChunk(Chunk* chunk);

private:
	/** Called by the worker thread (Launches into loop) */
	void RunWorker();
};

