#pragma once
#include "Common.h"
#include "Window.h"

#include "PerlinNoise.h"

#include "Chunk.h"
#include "DefaultMaterial.h"
#include "TerrainMaterial.h"
#include "TreeMaterial.h"

#include <thread>

#include <unordered_map>
#include <queue>

#include <mutex>


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

	/// Jobs that have succesfully completed go here, so they may perform any main thread logic
	std::queue<IChunkJob*> m_completedJobQueue;

	/// Where to currently load from
	ivec2 m_loadCentre;
	/// Where we were last syncing from
	ivec2 m_previousCentre;

	/// How close does a chunk have to be to be considered to work on
	int32 m_workRadius = 5;
	/// Chunks in this radius will be created, if they don't already exist
	int32 m_loadRadius = 4;
	/// How far away does a chunk have to be before being completely removed from memory
	int32 m_unloadRadius = 7;


	///
	/// Chunk pooling
	///
	uint32 m_poolSize;
	std::unordered_map<ivec2, Chunk*, ivec2_KeyFuncs> m_activeChunks;
	std::mutex m_chunkAccessMutex;
	std::queue<Chunk*> m_chunkPool;

	
	///
	/// Drawing, Visuals & Generation
	///
	PerlinNoise m_noiseGenerator;
	TerrainMaterial m_terrainMaterial;
	TreeMaterial m_treeMaterial;
	DefaultMaterial m_waterMaterial;


public:
	Terrain(Scene* scene, const uint32& seed = time(nullptr));
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
	/**
	* Callback when the trees should be rendered
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void RenderTrees(Window& window, const float& deltaTime);
	/**
	* Callback when the water should be rendered
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void RenderWater(Window& window, const float& deltaTime);

	//void RenderClouds(Window& window, const float& deltaTime);


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

	/**
	* Callback for when a chunk finishes generation
	* @param coords			The coords of the chunk that has just generated
	*/
	void NotifyChunkGeneration(const ivec2& coords);

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


	///
	/// Getters & Setters
	///
public:
	inline const PerlinNoise* GetNoiseGenerator() const { return &m_noiseGenerator; }
};

