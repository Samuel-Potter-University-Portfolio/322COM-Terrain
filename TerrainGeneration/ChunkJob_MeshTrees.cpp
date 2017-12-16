#include "ChunkJob_MeshTrees.h"
#include "Chunk.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\rotate_vector.hpp>


/**
* Recursively store tree that may have up to 4 branches attached to it
*/
struct TreeBranch
{
private:
	TreeBranch* m_subBranches[3]{ nullptr };
	uint32 m_subBranchCount;

	const float m_length;
	const float m_width;

public:
	TreeBranch(const float& length, const float& width, const float& lengthDecayFactor, const float& widthDecayAmount) :
		m_length(length), m_width(width)
	{
		m_subBranchCount = 0;

		if (width != 0.0f)
		{
			for (TreeBranch*& branch : m_subBranches)
			{
				const float subLength = length * lengthDecayFactor;
				float subWidth = width - widthDecayAmount;
				subWidth = subWidth < 0.0f ? 0.0f : subWidth;

				branch = new TreeBranch(subLength, subWidth, lengthDecayFactor, widthDecayAmount);
				++m_subBranchCount;
			}
		}
	}

	~TreeBranch() 
	{
		for (TreeBranch* branch : m_subBranches)
			if (branch != nullptr)
				delete branch;
	}

	/**
	* Generate the mesh for this tree, by treating this branch as the trunk
	*/
	void GenerateTreeMeshData(const vec3& offset, ChunkJob_MeshTrees& target)
	{
		const uint32 startIndex = target.m_vertices.size();

		// Generate base of tree
		{
			const int32 segments = 8;
			const float deltaAngle = 3.141592f * 2.0f / segments;


			// Build ring
			for (int32 i = 0; i <= segments; ++i)
			{
				const uint32 index = target.m_vertices.size();
				const vec3 P = vec3(cosf(deltaAngle * (i + 0)), 0, sinf(deltaAngle * (i + 0))) * m_width * 0.5f;

				target.m_vertices.emplace_back(offset + P);
				target.m_normals.emplace_back(P);
			}

			// No triangles need to be added in this step (Next steps will add)
		}

		// Generate recurisively using branches
		GenerateBranchMeshData(startIndex, offset, vec2(0, 0), target);
	}

public:
	/**
	* Generate the mesh for this tree, by treating this branch as just another branch
	*/
	void GenerateBranchMeshData(const uint32& previousCircle, const vec3& offset, const vec2& angle, ChunkJob_MeshTrees& target)
	{
		const vec3 direction = glm::rotateY(glm::rotateX(vec3(0, 1, 0), angle.x), angle.y);
		const vec3 circleOffset = offset + direction * m_length;
		const uint32 startIndex = target.m_vertices.size();


		// Build ring
		const int32 segments = 8;
		const float deltaAngle = 3.141592f * 2.0f / segments;

		for (int32 i = 0; i <= segments; ++i)
		{
			const uint32 index = target.m_vertices.size();
			const vec3 P = vec3(cosf(deltaAngle * (i + 0)), 0, sinf(deltaAngle * (i + 0))) * m_width * 0.5f;
			const vec3 rP = glm::rotateX(P, angle.x);

			target.m_vertices.emplace_back(circleOffset + rP);
			target.m_normals.emplace_back(rP);
		}

		// Create tris
		for (int32 i = 0; i < segments; ++i)
		{
			target.m_triangles.emplace_back(previousCircle + i + 0);
			target.m_triangles.emplace_back(startIndex + i + 0);
			target.m_triangles.emplace_back(startIndex + i + 1);

			target.m_triangles.emplace_back(previousCircle + i + 0);
			target.m_triangles.emplace_back(startIndex + i + 1);
			target.m_triangles.emplace_back(previousCircle + i + 1);
		}


		const float branchDeltaAngle = 3.141592f * 2.0f / m_subBranchCount;
		uint32 i = 0;

		for(TreeBranch* branch : m_subBranches)
			if (branch != nullptr)
			{
				const vec2 newAngle(
					angle.x + glm::radians(20.0f),
					angle.y + branchDeltaAngle * i++
				);
				
				branch->GenerateBranchMeshData(startIndex, circleOffset, newAngle, target);
			}
	}
};


ChunkJob_MeshTrees::ChunkJob_MeshTrees(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_MeshTrees::Execute() 
{
	AddTree(vec3(0, 35, 0));
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
	
	TreeBranch tree(5.0f, 1.0f, 0.95f, 0.3f);
	tree.GenerateTreeMeshData(totalOffset, *this);
}