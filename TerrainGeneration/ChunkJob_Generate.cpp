#include "ChunkJob_Generate.h"
#include "Chunk.h"
#include "Terrain.h"

#include "ChunkJob_MeshTerrain.h"
#include "PerlinNoise.h"


ChunkJob_Generate::ChunkJob_Generate(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_Generate::Execute() 
{
	Chunk& chunk = GetOwningChunk();
	const vec3 offset(chunk.GetCoords().x * CHUNK_SIZE, 0, chunk.GetCoords().y * CHUNK_SIZE);


	PerlinNoise noise;
	noise.SetSeed(1234);
	const float scale = 0.01f;

	// TEST GEN TERRAIN
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
			for (uint32 z = 0; z < CHUNK_SIZE; ++z)
			{
				const vec3 worldPos = offset + vec3(x, 0, z);
				const float v = noise.GetOctave(worldPos.x*scale, 0, worldPos.z*scale, 6, 0.3f);

				const uint32 h = v * CHUNK_HEIGHT;
				for (uint32 y = 0; y <= h; ++y)
					chunk.Set(x, y, z, y == h ? Voxel::Type::Grass : Voxel::Type::Dirt);
			}

	/*
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 y = 0; y < CHUNK_HEIGHT; ++y)
			for (uint32 z = 0; z < CHUNK_SIZE; ++z)
			{
				const vec3 worldPos = offset + vec3(x, y, z);
				const float v = noise.GetOctave(worldPos.x*scale, worldPos.y*scale, worldPos.z*scale, 1, 0.5f);
				if(v <= 0.5f)
					chunk.Set(x, y, z, Voxel::Type::Stone);
				else
					chunk.Set(x, y, z, Voxel::Type::Air);
			}
			*/

	/*
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
		{
			uint32 maxHeight = (std::cos((float)x / CHUNK_SIZE * 3.141592f * 2.0f) + 1.0f) * 0.1f * CHUNK_SIZE;
			
			for (uint32 y = 1; y < maxHeight + 1; ++y)
				chunk.Set(x, y, z, Voxel::Type::Grass);
		}

	chunk.Set(4, 1, 4, Voxel::Type::Sand);
	chunk.Set(5, 1, 4, Voxel::Type::Sand);
	chunk.Set(6, 1, 4, Voxel::Type::Stone);
	chunk.Set(4, 1, 5, Voxel::Type::Stone);
	chunk.Set(5, 1, 5, Voxel::Type::Sand);
	chunk.Set(6, 1, 5, Voxel::Type::Sand);
	chunk.Set(4, 1, 6, Voxel::Type::Stone);
	chunk.Set(5, 1, 6, Voxel::Type::Stone);
	chunk.Set(6, 1, 6, Voxel::Type::Sand);

	chunk.Set(5, 5, 5, Voxel::Type::Stone);
	chunk.Set(5, 4, 5, Voxel::Type::Sand);
	chunk.Set(5, 5, 6, Voxel::Type::Sand);
	chunk.Set(5, 4, 6, Voxel::Type::Stone);
	chunk.Set(6, 5, 5, Voxel::Type::Sand);
	chunk.Set(6, 4, 5, Voxel::Type::Stone);
	chunk.Set(6, 5, 6, Voxel::Type::Sand);
	chunk.Set(4, 4, 6, Voxel::Type::Stone);
	*/
}

void ChunkJob_Generate::OnComplete() 
{
	Chunk& chunk = GetOwningChunk();

	chunk.bAreVoxelsGenerated = true;
	chunk.GetTerrain()->NotifyChunkGeneration(chunk.GetCoords());

	// Build meshes
	chunk.QueueJob(new ChunkJob_MeshTerrain(&GetOwningChunk()));
}