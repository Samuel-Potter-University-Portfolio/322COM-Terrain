#include "Chunk.h"
#include "Logger.h"

#include "Terrain.h"
#include "ChunkJob_Generate.h"


Chunk::Chunk(Terrain* terrain) :
	m_terrain(terrain)
{
	m_terrainMesh = new Mesh;
}
Chunk::~Chunk()
{
	// Delete any jobs still in the queue
	while (m_jobQueue.size() != 0)
	{
		delete m_jobQueue.front();
		m_jobQueue.pop();
	}

	delete m_terrainMesh;
}

void Chunk::Alloc(const ivec2& coord) 
{
	m_chunkCoords = coord;
	//LOG("Alloc new chunk (%i %i)", m_chunkCoords.x, m_chunkCoords.y);

	bAreVoxelsGenerated = false;
	bIsTerrainMeshBuilt = false;


	// Try to generate terrain and features
	QueueJob(new ChunkJob_Generate(this));
}

void Chunk::Dealloc() 
{
	//LOG("Dealloc chunk (%i %i)", m_chunkCoords.x, m_chunkCoords.y);


	// Delete any jobs still in the queue
	while (m_jobQueue.size() != 0)
	{
		delete m_jobQueue.front();
		m_jobQueue.pop();
	}
}


Voxel::Type Chunk::Get(const int32& x, const int32& y, const int32& z) const 
{
	// Out of bounds (height) so return air
	if (y < 0 || y >= CHUNK_HEIGHT)
		return Voxel::Type::Air;


	// Out of bounds (width/depth) so check other chunks
	if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
		return m_terrain->Get(x + m_chunkCoords.x * CHUNK_SIZE, y, z + m_chunkCoords.y * CHUNK_SIZE);

	return m_voxels[GetIndex(x, y, z)]; 
}