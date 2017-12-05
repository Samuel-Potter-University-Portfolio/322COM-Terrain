#include "ChunkJob_MeshTerrain.h"
#include "Chunk.h"


#include "MarchingCubes.h"
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>




/**
* The appropriate hashing functions which are needed to use vec3 as a key
* https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
*/
struct vec3_KeyFuncs
{
	inline size_t operator()(const vec3& v)const
	{
		return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
	}

	inline bool operator()(const vec3& a, const vec3& b)const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};



/** Packet for passing around voxel density datas */
struct IsoPacket
{
	float		level;
	Voxel::Type	type;
};


/**
* Attempt to get the isolevel for this voxel (By calculating density)
* -Note: Will read other chunks, if necessiary
* @param chunk			The chunk we're trying to look at
* @param x,y,z			The local coordinates of the voxel to set
*/
static IsoPacket GetIsoData(const Chunk& chunk, const int32& x, const int32& y, const int32& z) 
{
	IsoPacket outPacket;
	outPacket.level = 0.0f;
	outPacket.type = chunk.Get(x, y, z);
	uint32 count = 0;

	// Calc densitiy base on whether it's a material or not
	if (!Voxel::IsMaterial(outPacket.type)) 
	{
		count = 1;
		if (!Voxel::IsMaterial(chunk.Get(x + 0, y + 0, z + 1))) ++count;
		if (!Voxel::IsMaterial(chunk.Get(x + 0, y + 0, z - 1))) ++count;
		if (!Voxel::IsMaterial(chunk.Get(x + 0, y + 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(chunk.Get(x + 0, y - 1, z + 0))) ++count;
		if (!Voxel::IsMaterial(chunk.Get(x + 1, y + 0, z + 0))) ++count;
		if (!Voxel::IsMaterial(chunk.Get(x - 1, y + 0, z + 0))) ++count;
		outPacket.level = (float)count / 6.0f;
	}
	else
	{
		count = 1;
		if (Voxel::IsMaterial(chunk.Get(x + 0, y + 0, z + 1))) ++count;
		if (Voxel::IsMaterial(chunk.Get(x + 0, y + 0, z - 1))) ++count;
		if (Voxel::IsMaterial(chunk.Get(x + 0, y + 1, z + 0))) ++count;
		if (Voxel::IsMaterial(chunk.Get(x + 0, y - 1, z + 0))) ++count;
		if (Voxel::IsMaterial(chunk.Get(x + 1, y + 0, z + 0))) ++count;
		if (Voxel::IsMaterial(chunk.Get(x - 1, y + 0, z + 0))) ++count;
		outPacket.level = (float)count / 6.0f;
	}

	return outPacket;
}

/**
* Interpolate between the iso data at these 2 vertices to get the correct mesh vertex
* @param chunk			The chunk we're trying to look at
* @param a,b			The local coordinates of the voxels to lerp between
* @param outPosition	The lerped position to use
* @param outType		The correect type to use
*/
static void LerpVertex(const Chunk& chunk, ivec3 a, ivec3 b, vec3& outPosition, Voxel::Type& outType)
{
	IsoPacket ap = GetIsoData(chunk, a.x, a.y, a.z);
	IsoPacket bp = GetIsoData(chunk, b.x, b.y, b.z);
	const float smoothness = 1.0f;
	vec3 af = a;
	vec3 bf = b;

	if (Voxel::IsMaterial(ap.type))
	{
		outPosition = af + (bf - af) * (0.5f + (ap.level - bp.level) * smoothness);
		outType = ap.type;
	}
	else
	{
		outPosition = bf + (af - bf) * (0.5f + (bp.level - ap.level) * smoothness);
		outType = bp.type;
	}
}



ChunkJob_MeshTerrain::ChunkJob_MeshTerrain(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_MeshTerrain::Execute() 
{
	const Chunk& chunk = GetOwningChunk();
	const vec3 chunkOffset = vec3(CHUNK_SIZE * chunk.GetCoords().x, 0, CHUNK_SIZE * chunk.GetCoords().y);


	std::unordered_map<vec3, uint32, vec3_KeyFuncs> vertexIndexLookup;
	vec3 edges[12];
	Voxel::Type types[12];

	// Extra triangles (Used for generating normal at the edge of chunk
	std::unordered_map<vec3, uint32, vec3_KeyFuncs> extraIndexLookup;
	std::vector<uint32> extraTriangles;
	std::vector<vec3> extraVertices;


	for (int32 x = -1; x <= CHUNK_SIZE; ++x)
		for (int32 y = 0; y < CHUNK_HEIGHT; ++y)
			for (int32 z = -1; z <= CHUNK_SIZE; ++z)
			{
				// Aborted, so stop execution here
				if (IsAborted())
					return;

				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (Voxel::IsMaterial(chunk.Get(x + 0, y + 0, z + 0))) caseIndex |= 1;
				if (Voxel::IsMaterial(chunk.Get(x + 1, y + 0, z + 0))) caseIndex |= 2;
				if (Voxel::IsMaterial(chunk.Get(x + 1, y + 0, z + 1))) caseIndex |= 4;
				if (Voxel::IsMaterial(chunk.Get(x + 0, y + 0, z + 1))) caseIndex |= 8;
				if (Voxel::IsMaterial(chunk.Get(x + 0, y + 1, z + 0))) caseIndex |= 16;
				if (Voxel::IsMaterial(chunk.Get(x + 1, y + 1, z + 0))) caseIndex |= 32;
				if (Voxel::IsMaterial(chunk.Get(x + 1, y + 1, z + 1))) caseIndex |= 64;
				if (Voxel::IsMaterial(chunk.Get(x + 0, y + 1, z + 1))) caseIndex |= 128;


				// Smooth edges based on density
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 1)
					LerpVertex(chunk, ivec3(x + 0, y + 0, z + 0), ivec3(x + 1, y + 0, z + 0), edges[0], types[0]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 2)
					LerpVertex(chunk, ivec3(x + 1, y + 0, z + 0), ivec3(x + 1, y + 0, z + 1), edges[1], types[1]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 4)
					LerpVertex(chunk, ivec3(x + 1, y + 0, z + 1), ivec3(x + 0, y + 0, z + 1), edges[2], types[2]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 8)
					LerpVertex(chunk, ivec3(x + 0, y + 0, z + 0), ivec3(x + 0, y + 0, z + 1), edges[3], types[3]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 16)
					LerpVertex(chunk, ivec3(x + 0, y + 1, z + 0), ivec3(x + 1, y + 1, z + 0), edges[4], types[4]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 32)
					LerpVertex(chunk, ivec3(x + 1, y + 1, z + 0), ivec3(x + 1, y + 1, z + 1), edges[5], types[5]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 64)
					LerpVertex(chunk, ivec3(x + 1, y + 1, z + 1), ivec3(x + 0, y + 1, z + 1), edges[6], types[6]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 128)
					LerpVertex(chunk, ivec3(x + 0, y + 1, z + 0), ivec3(x + 0, y + 1, z + 1), edges[7], types[7]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 256)
					LerpVertex(chunk, ivec3(x + 0, y + 0, z + 0), ivec3(x + 0, y + 1, z + 0), edges[8], types[8]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 512)
					LerpVertex(chunk, ivec3(x + 1, y + 0, z + 0), ivec3(x + 1, y + 1, z + 0), edges[9], types[9]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 1024)
					LerpVertex(chunk, ivec3(x + 1, y + 0, z + 1), ivec3(x + 1, y + 1, z + 1), edges[10], types[10]);
				if (MarchingCubes::CaseRequiredEdges[caseIndex] & 2048)
					LerpVertex(chunk, ivec3(x + 0, y + 0, z + 1), ivec3(x + 0, y + 1, z + 1), edges[11], types[11]);


				// Add all needed edges to form triangles
				int8* caseEdges = MarchingCubes::Cases[caseIndex];
				while (*caseEdges != -1)
				{
					int8 edge = *(caseEdges++);
					vec3 vert = chunkOffset + edges[edge];

					// Is extra triangle (Used for normal calculations)
					if (x == -1 || x == CHUNK_SIZE || z == -1 || z == CHUNK_SIZE)
					{
						// Reuse old vertex (Lets us do normal smoothing)
						auto it = extraIndexLookup.find(vert);
						if (it != extraIndexLookup.end())
							extraTriangles.emplace_back(it->second);

						else
						{
							const uint32 index = extraVertices.size();
							extraTriangles.emplace_back(index);
							extraVertices.emplace_back(vert);

							extraIndexLookup[vert] = index;
						}
					}

					// Is normal triangle
					else
					{
						// Reuse old vertex (Lets us do normal smoothing)
						auto it = vertexIndexLookup.find(vert);
						if (it != vertexIndexLookup.end())
							m_triangles.emplace_back(it->second);

						else
						{
							const uint32 index = m_vertices.size();
							m_triangles.emplace_back(index);
							m_vertices.emplace_back(vert);

							// Encode type as colour
							switch (types[edge])
							{
							case Voxel::Type::Grass:
								m_colours.emplace_back(1, 0, 0, 0);
								break;
							case Voxel::Type::Dirt:
								m_colours.emplace_back(0, 1, 0, 0);
								break;
							case Voxel::Type::Sand:
								m_colours.emplace_back(0, 0, 1, 0);
								break;
							case Voxel::Type::Stone:
								m_colours.emplace_back(0, 0, 0, 1);
								break;

							default:
								m_colours.emplace_back(0, 0, 0, 0);
								break;
							}

							vertexIndexLookup[vert] = index;
						}
					}

				}
			}


	// Make normals out of weighted triangles
	std::unordered_map<uint32, vec3> normalLookup;

	// Generate normals from triss
	for (uint32 i = 0; i < m_triangles.size(); i += 3)
	{
		// Aborted, so stop execution here
		if (IsAborted())
			return;

		uint32 ai = m_triangles[i];
		uint32 bi = m_triangles[i + 1];
		uint32 ci = m_triangles[i + 2];

		vec3 a = m_vertices[ai];
		vec3 b = m_vertices[bi];
		vec3 c = m_vertices[ci];


		// Normals are weighed based on the angle of the edges that connect that corner
		vec3 crossed = glm::cross(b - a, c - a);
		vec3 normal = glm::normalize(crossed);
		float area = crossed.length() * 0.5f;

		normalLookup[ai] += crossed * glm::angle(b - a, c - a);
		normalLookup[bi] += crossed * glm::angle(a - b, c - b);
		normalLookup[ci] += crossed * glm::angle(a - c, b - c);
	}
	// Repeat process for extra triangles
	for (uint32 i = 0; i < extraTriangles.size(); i += 3)
	{
		// Aborted, so stop execution here
		if (IsAborted())
			return;

		uint32 ai = extraTriangles[i];
		uint32 bi = extraTriangles[i + 1];
		uint32 ci = extraTriangles[i + 2];

		vec3 a = extraVertices[ai];
		vec3 b = extraVertices[bi];
		vec3 c = extraVertices[ci];


		// Normals are weighed based on the angle of the edges that connect that corner
		vec3 crossed = glm::cross(b - a, c - a);
		vec3 normal = glm::normalize(crossed);
		float area = crossed.length() * 0.5f;


		// Only store if the vertex is not an extra vertex
		{
			auto it = vertexIndexLookup.find(a);
			if (it != vertexIndexLookup.end())
				normalLookup[it->second] += crossed * glm::angle(b - a, c - a);
		}
		{
			auto it = vertexIndexLookup.find(b);
			if (it != vertexIndexLookup.end())
				normalLookup[it->second] += crossed * glm::angle(a - b, c - b);
		}
		{
			auto it = vertexIndexLookup.find(c);
			if (it != vertexIndexLookup.end())
				normalLookup[it->second] += crossed * glm::angle(a - c, b - c);
		}
	}

	// Put normals into vector
	m_normals.reserve(m_vertices.size());
	for (uint32 i = 0; i < m_vertices.size(); ++i)
		m_normals.emplace_back(normalLookup[i]);
}

void ChunkJob_MeshTerrain::OnComplete() 
{
	Mesh* mesh = GetOwningChunk().GetTerrainMesh();
	mesh->SetVertices(m_vertices);
	mesh->SetColours(m_colours);
	mesh->SetNormals(m_normals);
	mesh->SetTriangles(m_triangles);
	GetOwningChunk().bIsTerrainMeshBuilt = true;
}