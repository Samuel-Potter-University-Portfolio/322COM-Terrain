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
	/// Jobs & Working: Threading (Handles chunk generation/management jobs asynchronously)
	///
	std::thread* m_workerThread;
	bool bWorkerRunning = false;
	std::queue<IChunkJob*> m_activeJobQueue;
	std::queue<IChunkJob*> m_completedJobQueue;

	///
	/// Chunk pooling
	///
	const uint32 m_poolSize = 200;
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


	/**
	* Get the chunk coordinates for what this voxels is in
	* @param x,y,z			The world coordinates of the voxel to querry
	* @returns The XZ chunk coordinates for the owner
	*/
	inline ivec2 GetChunkCoords(const int32& x, const int32& y, const int32& z) const
	{
		ivec2 out;

		if (x < 0)
			out.x = -1 + (x + 1) / CHUNK_SIZE;
		else
			out.x = x / CHUNK_SIZE;

		if (z < 0)
			out.y = -1 + (z + 1) / CHUNK_SIZE;
		else
			out.y = z / CHUNK_SIZE;

		return out;
	}

	/**
	* Retreive a specific voxel from world coordinates
	* @param x,y,z			The world coordinates of the voxel to get
	*/
	Voxel::Type Get(const int32& x, const int32& y, const int32& z) const;

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

