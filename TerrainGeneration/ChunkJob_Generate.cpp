#include "ChunkJob_Generate.h"
#include "Chunk.h"

#include "ChunkJob_MeshTerrain.h"


ChunkJob_Generate::ChunkJob_Generate(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_Generate::Execute() 
{
	Chunk& chunk = GetOwningChunk();

	// TEST GEN TERRAIN
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
		{
			chunk.Set(x, 0, z, Voxel::Type::Stone);
			uint32 maxHeight = (std::sin((float)x / CHUNK_SIZE * 3.141592f * 2.0f) + 1.0f) * 0.1f * CHUNK_SIZE;

			if (x == 7 && z == 7)
				maxHeight = 8;

			for (uint32 y = 0; y < maxHeight + 1; ++y)
				chunk.Set(x, y, z, Voxel::Type::Stone);
		}

	chunk.Set(4, 1, 4, Voxel::Type::Stone);
	chunk.Set(5, 1, 4, Voxel::Type::Stone);
	chunk.Set(6, 1, 4, Voxel::Type::Stone);
	chunk.Set(4, 1, 5, Voxel::Type::Stone);
	chunk.Set(5, 1, 5, Voxel::Type::Stone);
	chunk.Set(6, 1, 5, Voxel::Type::Stone);
	chunk.Set(4, 1, 6, Voxel::Type::Stone);
	chunk.Set(5, 1, 6, Voxel::Type::Stone);
	chunk.Set(6, 1, 6, Voxel::Type::Stone);

	chunk.Set(5, 5, 5, Voxel::Type::Stone);
	chunk.Set(5, 4, 5, Voxel::Type::Stone);
	chunk.Set(5, 5, 6, Voxel::Type::Stone);
	chunk.Set(5, 4, 6, Voxel::Type::Stone);
	chunk.Set(6, 5, 5, Voxel::Type::Stone);
	chunk.Set(6, 4, 5, Voxel::Type::Stone);
	chunk.Set(6, 5, 6, Voxel::Type::Stone);
	chunk.Set(4, 4, 6, Voxel::Type::Stone);
}

void ChunkJob_Generate::OnComplete() 
{
	GetOwningChunk().bAreVoxelsGenerated = true;

	// Build meshes
	GetOwningChunk().QueueJob(new ChunkJob_MeshTerrain(&GetOwningChunk()));
}