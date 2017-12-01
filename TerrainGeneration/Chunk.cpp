#include "Chunk.h"
#include "Logger.h"

#include "MarchingCubes.h"


Chunk::Chunk(Terrain* terrain) :
	m_terrain(terrain)
{
	m_mesh = new Mesh;
}
Chunk::~Chunk()
{
	delete m_mesh;
}


vec3 Chunk::LerpVertex(ivec3 a, ivec3 b)
{
	// If b < a swap
	if ((b.x < a.x || b.y < a.z || b.z < a.z) && !(b.x > a.x || b.y > a.y || b.z > a.z))
	{
		ivec3 temp = a;
		a = b;
		b = temp;
	}

	Voxel::Packet ap = GetVoxelData(a.x, a.y, a.z);
	Voxel::Packet bp = GetVoxelData(b.x, b.y, b.z);
	const float isoLevel = 1.0f / 27.0f; // Effectively works out as smoothness
	const float bias = 0.00001f;


	if (std::fabs(ap.density - bp.density) > 0.0001f)
	{	
		vec3 af = a;
		vec3 bf = b;
		return af + (bf - af) / (bp.density - ap.density)*(isoLevel - ap.density);
	}
	// Too small
	else
		return a;
		
}

void Chunk::Alloc(const ivec2& coord) 
{
	m_chunkCoords = coord;
	LOG("Alloc new chunk (%i %i)", m_chunkCoords.x, m_chunkCoords.y);

	// TEST GEN TERRAIN
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
		{
			uint32 maxHeight = (std::cos((float)x / CHUNK_SIZE * 3.141592f * 2.0f) + 1.0f) * 0.25f * CHUNK_SIZE;

			if (x == 7 && z == 7)
				maxHeight = 8;

			for (uint32 y = 0; y < maxHeight + 1; ++y)
				Set(x, y, z, Voxel::Type::Stone);
		}



	// TEST BUILD MESH
	const vec3 chunkOffset = vec3(CHUNK_SIZE * m_chunkCoords.x, 0, CHUNK_SIZE * m_chunkCoords.y);
	vec3 edges[12];

	std::vector<uint32> triangles;
	std::vector<vec3> vertices;

	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 y = 0; y < CHUNK_HEIGHT; ++y)
			for (uint32 z = 0; z < CHUNK_SIZE; ++z)
			{
				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (Voxel::IsMaterial(Get(x + 0, y + 0, z + 0))) caseIndex |= 1;
				if (Voxel::IsMaterial(Get(x + 1, y + 0, z + 0))) caseIndex |= 2;
				if (Voxel::IsMaterial(Get(x + 1, y + 0, z + 1))) caseIndex |= 4;
				if (Voxel::IsMaterial(Get(x + 0, y + 0, z + 1))) caseIndex |= 8;
				if (Voxel::IsMaterial(Get(x + 0, y + 1, z + 0))) caseIndex |= 16;
				if (Voxel::IsMaterial(Get(x + 1, y + 1, z + 0))) caseIndex |= 32;
				if (Voxel::IsMaterial(Get(x + 1, y + 1, z + 1))) caseIndex |= 64;
				if (Voxel::IsMaterial(Get(x + 0, y + 1, z + 1))) caseIndex |= 128;


				// Smooth edges based on density
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 1)
					edges[0] = LerpVertex(ivec3(x + 0, y + 0, z + 0), ivec3(x + 1, y + 0, z + 0));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 2)
					edges[1] = LerpVertex(ivec3(x + 1, y + 0, z + 0), ivec3(x + 1, y + 0, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 4)
					edges[2] = LerpVertex(ivec3(x + 1, y + 0, z + 1), ivec3(x + 0, y + 0, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 8)
					edges[3] = LerpVertex(ivec3(x + 0, y + 0, z + 0), ivec3(x + 0, y + 0, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 16)
					edges[4] = LerpVertex(ivec3(x + 0, y + 1, z + 0), ivec3(x + 1, y + 1, z + 0));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 32)
					edges[5] = LerpVertex(ivec3(x + 1, y + 1, z + 0), ivec3(x + 1, y + 1, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 64)
					edges[6] = LerpVertex(ivec3(x + 1, y + 1, z + 1), ivec3(x + 0, y + 1, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 128)
					edges[7] = LerpVertex(ivec3(x + 0, y + 1, z + 0), ivec3(x + 0, y + 1, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 256)
					edges[8] = LerpVertex(ivec3(x + 0, y + 0, z + 0), ivec3(x + 0, y + 1, z + 0));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 512)
					edges[9] = LerpVertex(ivec3(x + 1, y + 0, z + 0), ivec3(x + 1, y + 1, z + 0));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 1024)
					edges[10] = LerpVertex(ivec3(x + 1, y + 0, z + 1), ivec3(x + 1, y + 1, z + 1));
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 2048)
					edges[11] = LerpVertex(ivec3(x + 0, y + 0, z + 1), ivec3(x + 0, y + 1, z + 1));


				// Add all needed edges to form triangles
				int8* caseEdges = MarchingCubes::Cases[caseIndex];
				while (*caseEdges != -1)
				{
					triangles.emplace_back(vertices.size());
					vertices.emplace_back(chunkOffset + edges[*(caseEdges++)]);
				}
			}

	m_mesh->SetVertices(vertices);
	m_mesh->SetTriangles(triangles);
	bIsMeshBuilt = true;
}

void Chunk::Dealloc() 
{
	bHasGenerated = false;
	bIsMeshBuilt = false;
	LOG("Dealloc chunk (%i %i)", m_chunkCoords.x, m_chunkCoords.y);
}


Voxel::Type Chunk::Get(const int32& x, const int32& y, const int32& z) const 
{
	// Out of bounds (height) so return air
	if (y < 0 || y >= CHUNK_HEIGHT)
		return Voxel::Type::Air;


	// Out of bounds (width/depth) so check other chunks
	if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
		// TODO - Checking other chunks
		return Voxel::Type::Air;

	return m_voxels[GetIndex(x, y, z)]; 
}
Voxel::Packet Chunk::GetVoxelData(const int32& x, const int32& y, const int32& z) 
{
	Voxel::Packet outPacket;
	outPacket.density = 0.0f;
	outPacket.type = Get(x, y, z);
	uint32 count = 0;

	// Calc densitiy base on whether it's a material or not
	if (!Voxel::IsMaterial(outPacket.type))
	{
		if (!Voxel::IsMaterial(Get(x + 0, y + 0, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y + 0, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y + 0, z - 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y + 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y + 1, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y + 1, z - 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y - 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y - 1, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 0, y - 1, z - 1))) ++count;

		if (!Voxel::IsMaterial(Get(x + 1, y + 0, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y + 0, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y + 0, z - 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y + 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y + 1, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y + 1, z - 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y - 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y - 1, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x + 1, y - 1, z - 1))) ++count;

		if (!Voxel::IsMaterial(Get(x - 1, y + 0, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y + 0, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y + 0, z - 1))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y + 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y + 1, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y + 1, z - 1))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y - 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y - 1, z + 1))) ++count;
		if (!Voxel::IsMaterial(Get(x - 1, y - 1, z - 1))) ++count;
		outPacket.density = (float)count / -27.0f;
	}
	else
	{
		if (Voxel::IsMaterial(Get(x + 0, y + 0, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y + 0, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y + 0, z - 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y + 1, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y + 1, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y + 1, z - 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y - 1, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y - 1, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 0, y - 1, z - 1))) ++count;

		if (Voxel::IsMaterial(Get(x + 1, y + 0, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y + 0, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y + 0, z - 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y + 1, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y + 1, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y + 1, z - 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y - 1, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y - 1, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x + 1, y - 1, z - 1))) ++count;

		if (Voxel::IsMaterial(Get(x - 1, y + 0, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y + 0, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y + 0, z - 1))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y + 1, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y + 1, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y + 1, z - 1))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y - 1, z + 0))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y - 1, z + 1))) ++count;
		if (Voxel::IsMaterial(Get(x - 1, y - 1, z - 1))) ++count;
		outPacket.density = (float)count / 27.0f;
	}

	return outPacket;
}