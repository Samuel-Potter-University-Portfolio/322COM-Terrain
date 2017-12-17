#include "ChunkJob_MeshWater.h"
#include "Chunk.h"

#include "Logger.h"


ChunkJob_MeshWater::ChunkJob_MeshWater(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_MeshWater::Execute()
{
	const Chunk& chunk = GetOwningChunk();
	const vec3 chunkOffset = vec3(CHUNK_SIZE * chunk.GetCoords().x, 0, CHUNK_SIZE * chunk.GetCoords().y);


	auto placeQuad = 
	[this, chunkOffset](const int32& x, const int32& y, const int32& z)
	{
		const uint32 index = m_vertices.size();

		m_vertices.emplace_back(chunkOffset + vec3(x, y, z) + vec3(-1, 0, -1) * 0.5f);
		m_vertices.emplace_back(chunkOffset + vec3(x, y, z) + vec3(1, 0, -1) * 0.5f);
		m_vertices.emplace_back(chunkOffset + vec3(x, y, z) + vec3(-1, 0, 1) * 0.5f);
		m_vertices.emplace_back(chunkOffset + vec3(x, y, z) + vec3(1, 0, 1) * 0.5f);

		m_triangles.emplace_back(index + 0);
		m_triangles.emplace_back(index + 2);
		m_triangles.emplace_back(index + 3);

		m_triangles.emplace_back(index + 0);
		m_triangles.emplace_back(index + 3);
		m_triangles.emplace_back(index + 1);
	};

	for (int32 x = 0; x < CHUNK_SIZE; ++x)
		for (int32 z = 0; z < CHUNK_SIZE; ++z)
		{
			Voxel::Type lastType = Voxel::Type::Air;


			for (int32 y = CHUNK_HEIGHT - 1; y >= 0; --y)
			{
				Voxel::Type type = chunk.Get(x, y, z);

				// Put quad facing upwards
				if (!Voxel::IsLiquid(lastType) && Voxel::IsLiquid(type))
				{
					placeQuad(x, y, z);

					for (int32 dx = -1; dx <= 1; ++dx)
						for (int32 dz = -1; dz <= 1; ++dz)
							if (!Voxel::IsLiquid(chunk.Get(x + dx, y, z + dz)))
								placeQuad(x + dx, y, z + dz);

				}

				lastType = type;
			}

		}

}

void ChunkJob_MeshWater::OnComplete()
{
	Mesh* mesh = GetOwningChunk().GetWaterMesh();
	mesh->SetVertices(m_vertices);
	mesh->SetTriangles(m_triangles);
	GetOwningChunk().bIsWaterMeshBuilt = true;
}