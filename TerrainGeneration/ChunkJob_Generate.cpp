#include "ChunkJob_Generate.h"
#include "Chunk.h"
#include "Terrain.h"

#include "ChunkJob_MeshTerrain.h"
#include "ChunkJob_MeshTrees.h"
#include "ChunkJob_MeshWater.h"
#include "PerlinNoise.h"



ChunkJob_Generate::ChunkJob_Generate(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_Generate::Execute() 
{
	Chunk& chunk = GetOwningChunk();
	const vec3 offset(chunk.GetCoords().x * CHUNK_SIZE, 0, chunk.GetCoords().y * CHUNK_SIZE);


	const PerlinNoise* noise = chunk.GetNoiseGenerator();
	const float biomeScale = 0.005f;

	const float scale = 0.02f;
	const float overhangScale = 0.01f;

	/// Biome info
	const float oceanEnd = 0.4f;
	const float woodsStart = 0.55f;

	const float beachHeight = 20;
	const uint32 waterHeight = 12;


	// Generate basic 2D height map
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
		{
			const vec3 worldPos2D = offset + vec3(x, 0, z);
			const float biomeValue = noise->GetOctave(worldPos2D.x*biomeScale, worldPos2D.z*biomeScale, 2, 0.8f);

			const float value = noise->GetOctave(worldPos2D.x*scale, worldPos2D.z*scale, 4, 0.3f);


			// Is in beach/ocean
			if (biomeValue <= oceanEnd)
			{
				const uint32 height = value * beachHeight;

				for (uint32 y = 0; y < height; y++)
					chunk.Set(x, y, z, Voxel::Type::Sand);
			}


			// In biome transition
			else if(biomeValue > oceanEnd && biomeValue < woodsStart)
			{
				// Normalize value for this region
				const float n = (biomeValue - oceanEnd) / (woodsStart - oceanEnd);

				// LERP between expected values for both biomes
				const float beachH = beachHeight + value * (CHUNK_HEIGHT - beachHeight);
				const float woodsH = value * beachHeight;

				const uint32 height = beachH * (n) + woodsH * (1.0f - n);

				for (uint32 y = 0; y < height; y++)
				{
					// Set to beach
					if (n < 0.3f)
						chunk.Set(x, y, z, Voxel::Type::Sand);

					// Set to woods
					else
					{
						const vec3 worldPos3D = offset + vec3(x, y, z);
						const float overhangValue = noise->GetOctave(worldPos3D.x*overhangScale, worldPos3D.y*overhangScale, worldPos3D.z*overhangScale, 4, 0.8f);


						float overhangThreshold = 0.0f;
						if (y < beachHeight)
							overhangThreshold = 1.0f;
						else
							overhangThreshold = 1.0f - glm::min((float)(y - beachHeight) / 20.0f, 1.0f) * 0.5f;

						if (overhangValue <= overhangThreshold)
							chunk.Set(x, y, z, Voxel::Type::Stone);
					}
				}
			}


			// Is in woodlands
			else
			{
				const uint32 height = beachHeight + value * (CHUNK_HEIGHT - beachHeight);

				for (uint32 y = 0; y < height; y++)
				{
					const vec3 worldPos3D = offset + vec3(x, y, z);
					const float overhangValue = noise->GetOctave(worldPos3D.x*overhangScale, worldPos3D.y*overhangScale, worldPos3D.z*overhangScale, 4, 0.8f);


					float overhangThreshold = 0.0f;
					if (y < beachHeight)
						overhangThreshold = 1.0f;
					else
						overhangThreshold = 1.0f - glm::min((float)(y - beachHeight) / 20.0f, 1.0f) * 0.5f;

					if(overhangValue <= overhangThreshold)
						chunk.Set(x, y, z, Voxel::Type::Stone);
				}
			}
			

			// Place sand at bottom
			chunk.Set(x, 0, z, Voxel::Type::Sand);
		}

	
	// Place grass on top of the surface
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
		{
			for (int32 y = CHUNK_HEIGHT - 1; y >= 0; --y)
			{
				Voxel::Type type = chunk.Get(x, y, z);

				if (type != Voxel::Type::Air)
				{
					if (type == Voxel::Type::Stone)
					{
						chunk.Set(x, y, z, Voxel::Type::Grass);

						for (int32 dy = y - 1; dy >= y - 3 && dy >= 0; --dy)
							chunk.Set(x, dy, z, Voxel::Type::Dirt);
					}
					break;
				}
			}
		}


	// Place water on top of sand
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
		{
			for (int32 y = CHUNK_HEIGHT - 1; y >= 0; --y)
			{
				Voxel::Type type = chunk.Get(x, y, z);

				if (type != Voxel::Type::Air)
				{
					if (type == Voxel::Type::Sand)
					{
						// Place water
						for (int32 cy = 0; cy <= waterHeight; ++cy)
							if (chunk.Get(x, cy, z) == Voxel::Type::Air)
								chunk.Set(x, cy, z, Voxel::Type::Water);
					}
					break;
				}
			}
		}
}

void ChunkJob_Generate::OnComplete() 
{
	Chunk& chunk = GetOwningChunk();

	chunk.bAreVoxelsGenerated = true;
	chunk.GetTerrain()->NotifyChunkGeneration(chunk.GetCoords());

	// Build meshes
	chunk.QueueJob(new ChunkJob_MeshTerrain(&chunk));
	chunk.QueueJob(new ChunkJob_MeshTrees(&chunk));
	chunk.QueueJob(new ChunkJob_MeshWater(&chunk));
}