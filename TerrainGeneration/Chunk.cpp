#include "Chunk.h"
#include "Terrain.h"
#include "Logger.h"

#include <algorithm>

#include "Terrain.h"
#include "ChunkJob_Generate.h"
#include "ChunkJob_MeshTerrain.h"
#include "ChunkJob_MeshTrees.h";


Chunk::Chunk(Terrain* terrain) :
	m_terrain(*terrain)
{
	m_terrainMesh = new Mesh;
	m_treeMesh = new LeveledMesh;
	m_waterMesh = new Mesh;

	m_treeMesh->AddLoDs(TREE_LOD_COUNT);
	m_treeMesh->SetLoDRange(150.0f);
}
Chunk::~Chunk()
{
	// Delete any jobs still in the queue
	while (m_pendingJobQueue.size() != 0)
	{
		delete m_pendingJobQueue.front();
		m_pendingJobQueue.pop();
	}

	delete m_terrainMesh;
	delete m_treeMesh;
	delete m_waterMesh;
}

void Chunk::Alloc(const ivec2& coord) 
{
	m_chunkCoords = coord;
	//LOG("Alloc new chunk (%i %i)", m_chunkCoords.x, m_chunkCoords.y);

	bAreVoxelsGenerated = false;
	bIsTerrainMeshBuilt = false;
	bIsTreeMeshBuilt = false;
	bIsWaterMeshBuilt = false;


	// Try to generate terrain and features
	QueueJob(new ChunkJob_Generate(this));
}

void Chunk::Dealloc() 
{
	//LOG("Dealloc chunk (%i %i)", m_chunkCoords.x, m_chunkCoords.y);

	// Abort any active jobs
	for (IChunkJob* job : m_activeJobs)
		job->Abort();
	m_activeJobs.clear();

	// Delete any jobs still in the queue
	while (m_pendingJobQueue.size() != 0)
	{
		delete m_pendingJobQueue.front();
		m_pendingJobQueue.pop();
	}

	// Clear voxels
	for (Voxel::Type& v : m_voxels)
		v = Voxel::Type::Air;
}


Voxel::Type Chunk::Get(const int32& x, const int32& y, const int32& z) const 
{
	// Out of bounds (height) so return air
	if (y < 0 || y >= CHUNK_HEIGHT)
		return Voxel::Type::Air;


	// Out of bounds (width/depth) so check other chunks
	if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
		return m_terrain.Get(x + m_chunkCoords.x * CHUNK_SIZE, y, z + m_chunkCoords.y * CHUNK_SIZE);

	return m_voxels[GetIndex(x, y, z)]; 
}


IChunkJob* Chunk::GetQueuedJob() 
{ 
	IChunkJob* job = m_pendingJobQueue.front(); 
	m_pendingJobQueue.pop(); 

	m_activeJobs.push_back(job); 
	return job;
}
void Chunk::OnJobCompletion(IChunkJob* job) 
{
	for (auto it = m_activeJobs.begin(); it != m_activeJobs.end(); ++it)
	{
		if ((*it) == job)
		{
			m_activeJobs.erase(it);
			return;
		}
	}
}

void Chunk::OnAdjacentChunkGenerate() 
{
	// Rebuild mesh as this one may be out of date
	if(bIsTerrainMeshBuilt)
		QueueJob(new ChunkJob_MeshTerrain(this));
}

const PerlinNoise* Chunk::GetNoiseGenerator() const
{
	return m_terrain.GetNoiseGenerator();
}