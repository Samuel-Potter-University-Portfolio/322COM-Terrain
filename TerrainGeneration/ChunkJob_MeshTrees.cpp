#include "ChunkJob_MeshTrees.h"
#include "Chunk.h"
#include "PerlinNoise.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\rotate_vector.hpp>


/**
* Recursively store tree that may have up to 4 branches attached to it
*/
struct TreeBranch
{
private:
	TreeBranch* m_subBranches[4]{ nullptr };
	uint32 m_subBranchCount;

	const vec3 m_location;
	const vec2 m_angle;

	const float m_length;
	const float m_width;

public:
	TreeBranch(const PerlinNoise* noise, const vec3& location, const vec2& angle, const float& length, const float& width, const float& lengthDecayFactor, const float& widthDecayAmount) :
		m_location(location), m_angle(angle),
		m_length(length), m_width(width)
	{
		m_subBranchCount = 0;

		if (width != 0.0f)
		{
			// Generate sub branches by decaying current settings and adding variation
			for (TreeBranch*& branch : m_subBranches)
			{
				const vec3 direction = glm::rotateY(glm::rotateX(vec3(0, 1, 0), angle.x), angle.y);
				const vec3 subLocation = location + direction * length;

				const float noiseScale = 1.0f;
				const float v0 = noise->Get01(subLocation.x * noiseScale + m_subBranchCount, (subLocation.y + m_subBranchCount) * noiseScale, subLocation.z * noiseScale + m_subBranchCount);
				const float v1 = noise->Get01(subLocation.y * noiseScale + m_subBranchCount, (subLocation.z + m_subBranchCount) * noiseScale, subLocation.x * noiseScale + m_subBranchCount);
				const float v2 = noise->Get01(subLocation.x * noiseScale + m_subBranchCount, (subLocation.z + m_subBranchCount) * noiseScale, subLocation.x * noiseScale + m_subBranchCount);
				if (m_subBranchCount != 0 && v1 < 0.33f)
					continue;

				const float subLength = length * lengthDecayFactor * (0.8f + 0.2f * v2);

				
				float subWidth = width - widthDecayAmount - (0.5f * v1);
				subWidth = subWidth < 0.1f ? 0.0f : subWidth;
				if (v0 < 0.33f)
					subWidth = 0.0f;


				const float branchDeltaAngle = 3.141592f * 2.0f / 4.0f;
				const vec2 subAngle(
					angle.x + glm::radians(-30.0f + 20.0f * v2),
					angle.y + branchDeltaAngle * m_subBranchCount + glm::radians(-100.0f + 50.0f * v1)
				);


				branch = new TreeBranch(noise, subLocation, subAngle, subLength, subWidth, lengthDecayFactor, widthDecayAmount);
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
	void GenerateTreeMeshData(ChunkJob_MeshTrees& target)
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

				target.m_vertices.emplace_back(m_location + P);
				target.m_normals.emplace_back(P);
			}

			// No triangles need to be added in this step (Next steps will add)
		}

		// Generate recurisively using branches
		GenerateBranchMeshData(startIndex, target);
	}

public:
	/**
	* Generate the mesh for this tree, by treating this branch as just another branch
	*/
	void GenerateBranchMeshData(const uint32& previousCircle, ChunkJob_MeshTrees& target)
	{
		const vec3 direction = glm::rotateY(glm::rotateX(vec3(0, 1, 0), m_angle.x), m_angle.y);
		const vec3 circleOffset = m_location + direction * m_length;
		const uint32 startIndex = target.m_vertices.size();


		// Build ring
		const int32 segments = 8;
		const float deltaAngle = 3.141592f * 2.0f / segments;

		for (int32 i = 0; i <= segments; ++i)
		{
			const uint32 index = target.m_vertices.size();
			const vec3 P = vec3(cosf(deltaAngle * (i + 0)), 0, sinf(deltaAngle * (i + 0))) * m_width * 0.5f;
			const vec3 rP = glm::rotateX(P, m_angle.x);

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


		// Create sub branches
		for(TreeBranch* branch : m_subBranches)
			if (branch != nullptr)
				branch->GenerateBranchMeshData(startIndex, target);
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
	
	TreeBranch tree(chunk.GetNoiseGenerator(), totalOffset, vec2(0,0), 4.0f, 1.0f, 0.95f, 0.3f);
	tree.GenerateTreeMeshData(*this);
}