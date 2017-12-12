#include "ChunkJob_MeshTrees.h"
#include "Chunk.h"



ChunkJob_MeshTrees::ChunkJob_MeshTrees(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_MeshTrees::Execute() 
{
	AddTree(vec3(0, 0, 0));
}

void ChunkJob_MeshTrees::OnComplete() 
{
	Mesh* mesh = GetOwningChunk().GetTreeMesh();
	mesh->SetVertices(m_vertices);
	mesh->SetNormals(m_normals);
	mesh->SetTriangles(m_triangles);
	GetOwningChunk().bIsTreeMeshBuilt = true;
}

void ChunkJob_MeshTrees::AddTree(const vec3& offset) 
{
	const Chunk& chunk = GetOwningChunk();
	const vec3 totalOffset = vec3(CHUNK_SIZE * chunk.GetCoords().x, 0, CHUNK_SIZE * chunk.GetCoords().y) + offset;

	const int32 segments = 8;
	const float deltaAngle = 3.141592f * 2.0f / segments;

	for (int32 i = 0; i < segments; ++i)
	{
		const uint32 index = m_vertices.size();
		const vec3 a = vec3(cosf(deltaAngle * (i + 0)), 0, sinf(deltaAngle * (i + 0)));
		const vec3 b = vec3(cosf(deltaAngle * (i + 1)), 0, sinf(deltaAngle * (i + 1)));

		m_vertices.emplace_back(totalOffset + a);
		m_vertices.emplace_back(totalOffset + b);
		m_vertices.emplace_back(totalOffset + a + vec3(0, 5, 0));
		m_vertices.emplace_back(totalOffset + b + vec3(0, 5, 0));

		m_normals.emplace_back(a);
		m_normals.emplace_back(b);
		m_normals.emplace_back(a);
		m_normals.emplace_back(b);

		m_triangles.emplace_back(index + 0);
		m_triangles.emplace_back(index + 3);
		m_triangles.emplace_back(index + 1);

		m_triangles.emplace_back(index + 0);
		m_triangles.emplace_back(index + 2);
		m_triangles.emplace_back(index + 3);
	}
}