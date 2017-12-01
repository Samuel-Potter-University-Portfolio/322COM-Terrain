#include "Chunk.h"
#include "MarchingCubes.h"


Chunk::Chunk(Terrain* terrain) :
	m_terrain(terrain)
{
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
			Set(x, 0, z, Voxel::Type::Stone);

	Set(5, 7, 5, Voxel::Type::Stone);
	Set(4, 7, 5, Voxel::Type::Stone);
	Set(5, 6, 5, Voxel::Type::Stone);
	Set(4, 6, 5, Voxel::Type::Stone);
	Set(3, 7, 5, Voxel::Type::Stone);
	Set(5, 6, 5, Voxel::Type::Stone);

	// TEST BUILD MEsH
	m_mesh = new Mesh;

	std::vector<uint32> triangles;
	std::vector<vec3> vertices;

	for (uint32 x = 0; x < CHUNK_SIZE - 1; ++x)
		for (uint32 y = 0; y < CHUNK_HEIGHT - 1; ++y)
			for (uint32 z = 0; z < CHUNK_SIZE - 1; ++z)
			{
				uint8 caseIndex = 0;
				if (Get(x + 0, y + 0, z + 0) != Voxel::Type::Air) caseIndex |= 1;
				if (Get(x + 1, y + 0, z + 0) != Voxel::Type::Air) caseIndex |= 2;
				if (Get(x + 1, y + 0, z + 1) != Voxel::Type::Air) caseIndex |= 4;
				if (Get(x + 0, y + 0, z + 1) != Voxel::Type::Air) caseIndex |= 8;
				if (Get(x + 0, y + 1, z + 0) != Voxel::Type::Air) caseIndex |= 16;
				if (Get(x + 1, y + 1, z + 0) != Voxel::Type::Air) caseIndex |= 32;
				if (Get(x + 1, y + 1, z + 1) != Voxel::Type::Air) caseIndex |= 64;
				if (Get(x + 0, y + 1, z + 1) != Voxel::Type::Air) caseIndex |= 128;

				int8* edges = MarchingCubes::Cases[caseIndex];
				while (*edges != -1)
				{
					triangles.emplace_back(vertices.size());
					vertices.emplace_back(vec3(x, y, z) + MarchingCubes::Edges[*(edges++)]);
				}
			}

	m_mesh->SetVertices(vertices);
	m_mesh->SetTriangles(triangles);
}

Chunk::~Chunk()
{
	delete m_mesh;
}
