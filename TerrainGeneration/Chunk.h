#pragma once
#include "Common.h"

#include "IChunkJob.h"
#include "Mesh.h"
#include "LeveledMesh.h"

#include <queue>
#include <list>
#include <array>


#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 64


class Terrain;



namespace Voxel 
{
	/** Supported voxel types */
	enum class Type : uint8 
	{
		Air = 0,
		Grass,
		Dirt,
		Sand,
		Stone,

		Water,
	};

	/** Is this type considered a material i.e. to be built with marching cubes */
	inline bool IsMaterial(const Type& t) { return t != Type::Air && t != Type::Water; }

	/** Is this type considered a liquid */
	inline bool IsLiquid(const Type& t) { return t == Type::Water; }
}



/**
* Holds all the voxel information for a small chunk of the terrain
*/
class Chunk
{
private:
	///
	/// Terrain vars
	///
	Terrain& m_terrain;
	std::array<Voxel::Type, CHUNK_HEIGHT * CHUNK_SIZE * CHUNK_SIZE> m_voxels{ Voxel::Type::Air };
	ivec2 m_chunkCoords;

	///
	/// Rendering/Generating vars
	///
	Mesh* m_terrainMesh = nullptr;
	LeveledMesh* m_treeMesh = nullptr;
	Mesh* m_waterMesh = nullptr;

	friend class ChunkJob_Generate;
	friend class ChunkJob_MeshTerrain;
	friend class ChunkJob_MeshTrees;
	friend class ChunkJob_MeshWater;
	bool bAreVoxelsGenerated = false;
	bool bIsTerrainMeshBuilt = false;
	bool bIsTreeMeshBuilt = false;
	bool bIsWaterMeshBuilt = false;

	///
	/// Jobs
	///
	std::queue<IChunkJob*> m_pendingJobQueue; 
	std::list<IChunkJob*> m_activeJobs;

public:
	Chunk(Terrain* terrain);
	~Chunk();


	/**
	* Callback for when this chunk comes into usage
	* (Called when chunk is used from object pool)
	* @param coord			The new chunk coord that this will use
	*/
	void Alloc(const ivec2& coord);
	/**
	* Callback for when this chunk goes out of usage (i.e. goes back into the pool)
	* (Called when chunk goes back into object pool)
	*/
	void Dealloc();


private:
	/// Convect XYZ to a valid array index
	inline static int32 GetIndex(const int32& x, const int32& y, const int32& z) { return x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE; }

public:
	/** Callback for when an adjacent chunk has finished generation */
	void OnAdjacentChunkGenerate();

	/**
	* Sets a specific voxel from local coordinates
	* @param x,y,z			The local coordinates of the voxel to set
	* @param type			The type this voxel should be
	*/
	inline void Set(const int32& x, const int32& y, const int32& z, const Voxel::Type& type) 
	{
#ifdef _DEBUG
		if (!(x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_SIZE))
			throw std::exception("Setting invalid voxel coordinate");
#endif
		m_voxels[GetIndex(x, y, z)] = type; 
	}
	/**
	* Retreive a specific voxel from local coordinates
	* -Note: Will read other chunks, if necessiary
	* @param x,y,z			The local coordinates of the voxel to set
	*/
	Voxel::Type Get(const int32& x, const int32& y, const int32& z) const;


public:
	/// Are they any jobs for this chunk queued currently
	inline bool HasQueuedJob() const { return m_pendingJobQueue.size() != 0; }

	/** Pops the next queued job */
	IChunkJob* GetQueuedJob();

	/**
	* Callback for when this job completes it's execution
	* @param job			The newly completed job
	*/
	void OnJobCompletion(IChunkJob* job);

protected:
	/// Queues up this job for execution
	inline void QueueJob(IChunkJob* job) { m_pendingJobQueue.emplace(job); }


	///
	/// Getters & Setters
	///
public:
	inline Terrain* GetTerrain() const { return &m_terrain; }
	const class PerlinNoise* GetNoiseGenerator() const;

	inline ivec2 GetCoords() const { return m_chunkCoords; }
	inline Mesh* GetTerrainMesh() const { return m_terrainMesh; }
	inline LeveledMesh* GetTreeMesh() const { return m_treeMesh; }
	inline Mesh* GetWaterMesh() const { return m_waterMesh; }

	inline bool HasVoxelsGenerated() const { return bAreVoxelsGenerated; }
	inline bool IsTerrainMeshBuilt() const { return bIsTerrainMeshBuilt; }
	inline bool IsTreeMeshBuilt() const { return bIsTreeMeshBuilt; }
	inline bool IsWaterMeshBuilt() const { return bIsWaterMeshBuilt; }
};

