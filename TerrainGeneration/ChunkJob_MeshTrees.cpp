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
	TreeBranch* m_subBranches[3]{ nullptr };
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
		uint32 i = 0;

		if (width != 0.0f)
		{
			// Generate sub branches by decaying current settings and adding variation
			for (TreeBranch*& branch : m_subBranches)
			{
				i++;
				const vec3 direction = glm::rotateY(glm::rotateX(vec3(0, 1, 0), angle.x), angle.y);
				const vec3 subLocation = location + direction * length;

				// Generate a few random values to use
				const float noiseScale = 1.0f;
				const float v0 = noise->Get01(subLocation.x * noiseScale + i, (subLocation.y + i) * noiseScale, subLocation.z * noiseScale + i);
				const float v1 = noise->Get01(subLocation.y * noiseScale + i, (subLocation.z + i) * noiseScale, subLocation.x * noiseScale + i);
				const float v2 = noise->Get01(subLocation.x * noiseScale + i, (subLocation.z + i) * noiseScale, subLocation.x * noiseScale + i);


				// Miss branches every now and then
				if (m_subBranchCount != 0 && v1 < 0.33f)
					continue;


				// Vary length
				const float subLength = length * lengthDecayFactor * (0.8f + 0.2f * v2);


				// Vary width
				float subWidth = width - widthDecayAmount - (0.5f * v1);
				subWidth = subWidth < 0.1f ? 0.0f : subWidth;
				if (v0 < 0.33f)
					subWidth = 0.0f;


				// Rotate branches and vary slightly
				const float branchDeltaAngle = 3.141592f * 2.0f / 3.0f;
				const vec2 subAngle(
					glm::clamp(angle.x + glm::radians(-60.0f + 120.0f * v2), glm::radians(-88.0f), glm::radians(88.0f)),
					angle.y + branchDeltaAngle * i + glm::radians(-100.0f + 50.0f * v1)
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
	* @param lod						The current level of detail which the mesh is being built for
	* @param target						Where to store the mesh data
	*/
	void GenerateTreeMeshData(const uint32& lod, ChunkJob_MeshTrees& target)
	{
		const uint32 startIndex = target.m_vertices[lod].size();

		// Generate base of tree
		{
			const int32 segments = std::max(8 - lod * 2, 3U);
			const float deltaAngle = 3.141592f * 2.0f / segments;


			// Build ring
			for (int32 i = 0; i <= segments; ++i)
			{
				const uint32 index = target.m_vertices[lod].size();
				const vec3 P = vec3(cosf(deltaAngle * (i + 0)), 0, sinf(deltaAngle * (i + 0))) * m_width * 0.5f;

				target.m_vertices[lod].emplace_back(m_location + P);
				target.m_normals[lod].emplace_back(P);
				target.m_uvs[lod].emplace_back(vec2((float)i / (float)segments * m_width, 0.0f));

				target.m_extraData0[lod].emplace_back(vec2(
					0,		// Texture type
					0		// Sway weight
				));
			}

			// No triangles need to be added in this step (Next steps will add)
		}

		// Generate recurisively using branches
		GenerateBranchMeshData(lod, 0, startIndex, 0.0f, target);
	}

public:

	/**
	* Generate the mesh for this tree, by treating this branch as just another branch
	* @param lod						The current level of detail which the mesh is being built for
	* @param depth						How deep into the tree are we
	* @param previousCircle				The index that the previous ring of vertices started at
	* @param totalLength				The total length of the tree up to this point
	* @param target						Where to store the mesh data
	*/
	void GenerateBranchMeshData(const uint32& lod, const uint32& depth, const uint32& previousCircle, const float& totalLength, ChunkJob_MeshTrees& target)
	{
		const vec3 direction = glm::rotateY(glm::rotateX(vec3(0, 1, 0), m_angle.x), m_angle.y);
		const vec3 circleOffset = m_location + direction * m_length;
		uint32 startIndex = target.m_vertices[lod].size();


		// Only build specific branches at certain LoD
		if (lod == 0 || (depth % (lod + 1) == 0 || m_width == 0.0f))
		{
			// Build ring
			const int32 segments = std::max(8 - lod * 2, 3U);
			const float deltaAngle = 3.141592f * 2.0f / segments;

			for (int32 i = 0; i <= segments; ++i)
			{
				const uint32 index = target.m_vertices[lod].size();
				const vec3 P = vec3(cosf(deltaAngle * (i + 0)), 0, sinf(deltaAngle * (i + 0))) * m_width * 0.5f;
				const vec3 rP = glm::rotateX(P, -m_angle.x);

				target.m_vertices[lod].emplace_back(circleOffset + P);
				target.m_normals[lod].emplace_back(P);
				target.m_uvs[lod].emplace_back(vec2((float)i / (float)segments * m_width, totalLength + m_length));
				target.m_extraData0[lod].emplace_back(vec2(
					0,										// Texture type
					glm::max(1.0f - m_width * 0.5f, 0.0f)	// Sway weight
					));
			}

			// Create tris
			for (int32 i = 0; i < segments; ++i)
			{
				target.m_triangles[lod].emplace_back(previousCircle + i + 0);
				target.m_triangles[lod].emplace_back(startIndex + i + 0);
				target.m_triangles[lod].emplace_back(startIndex + i + 1);

				target.m_triangles[lod].emplace_back(previousCircle + i + 0);
				target.m_triangles[lod].emplace_back(startIndex + i + 1);
				target.m_triangles[lod].emplace_back(previousCircle + i + 1);
			}
		}
		else
			startIndex = previousCircle;


		// Add leaves, if at end
		if (m_width == 0)
			GenerateLeaves(lod, totalLength, target);

		// Add branches, if not at end
		else
		{
			// Create sub branches
			for (TreeBranch* branch : m_subBranches)
				if (branch != nullptr)
					branch->GenerateBranchMeshData(lod, depth + 1, startIndex, totalLength + m_length, target);
		}
	}

	/**
	* Generate the leaves for the end of a branch
	* @param lod						The current level of detail which the mesh is being built for
	* @param totalLength				The total length of the tree up to this point
	* @param target						Where to store the mesh data
	*/
	void GenerateLeaves(const uint32& lod, const float& totalLength, ChunkJob_MeshTrees& target)
	{
		const vec3 direction = glm::rotateY(glm::rotateX(vec3(0, 1, 0), m_angle.x), m_angle.y);
		const vec3 offset = m_location + direction * m_length;

		const vec2 extraData0(
			1,		// Texture type
			1		// Sway weight
			);


		const uint32 index = target.m_vertices[lod].size();
		const float leafSize = totalLength * 0.3f;
		const float leafSag = 0.4f + totalLength * 0.2f;

		// Create pyramid
		if (lod != TREE_LOD_COUNT - 1)
		{
			target.m_vertices[lod].emplace_back(offset + vec3(-1, 0, -1) * leafSize + vec3(0, -leafSag, 0));
			target.m_vertices[lod].emplace_back(offset + vec3(1, 0, -1) * leafSize + vec3(0, -leafSag, 0));
			target.m_vertices[lod].emplace_back(offset + vec3(-1, 0, 1) * leafSize + vec3(0, -leafSag, 0));
			target.m_vertices[lod].emplace_back(offset + vec3(1, 0, 1) * leafSize + vec3(0, -leafSag, 0));
			target.m_vertices[lod].emplace_back(offset + vec3(0, 0, 0) * leafSize);

			target.m_normals[lod].emplace_back(glm::normalize(vec3(-1, 0.5f, -1)));
			target.m_normals[lod].emplace_back(glm::normalize(vec3(1, 0.5f, -1)));
			target.m_normals[lod].emplace_back(glm::normalize(vec3(-1, 0.5f, -1)));
			target.m_normals[lod].emplace_back(glm::normalize(vec3(1, 0.5f, 1)));
			target.m_normals[lod].emplace_back(vec3(0, 1, 0));

			target.m_uvs[lod].emplace_back(vec2(0, 0));
			target.m_uvs[lod].emplace_back(vec2(1, 0));
			target.m_uvs[lod].emplace_back(vec2(0, 1));
			target.m_uvs[lod].emplace_back(vec2(1, 1));
			target.m_uvs[lod].emplace_back(vec2(0.5f, 0.5f));

			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);


			// Make faces 2 sided
			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 4);
			target.m_triangles[lod].emplace_back(index + 1);
			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 1);
			target.m_triangles[lod].emplace_back(index + 4);

			target.m_triangles[lod].emplace_back(index + 1);
			target.m_triangles[lod].emplace_back(index + 4);
			target.m_triangles[lod].emplace_back(index + 3);
			target.m_triangles[lod].emplace_back(index + 1);
			target.m_triangles[lod].emplace_back(index + 3);
			target.m_triangles[lod].emplace_back(index + 4);

			target.m_triangles[lod].emplace_back(index + 4);
			target.m_triangles[lod].emplace_back(index + 3);
			target.m_triangles[lod].emplace_back(index + 2);
			target.m_triangles[lod].emplace_back(index + 4);
			target.m_triangles[lod].emplace_back(index + 2);
			target.m_triangles[lod].emplace_back(index + 3);

			target.m_triangles[lod].emplace_back(index + 4);
			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 2);
			target.m_triangles[lod].emplace_back(index + 4);
			target.m_triangles[lod].emplace_back(index + 2);
			target.m_triangles[lod].emplace_back(index + 0);
		}

		// Flat quad
		else
		{
			target.m_vertices[lod].emplace_back(offset + vec3(-1, 0, -1) * leafSize);
			target.m_vertices[lod].emplace_back(offset + vec3(1, 0, -1) * leafSize);
			target.m_vertices[lod].emplace_back(offset + vec3(-1, 0, 1) * leafSize);
			target.m_vertices[lod].emplace_back(offset + vec3(1, 0, 1) * leafSize);

			target.m_normals[lod].emplace_back(glm::normalize(vec3(-1, 0.5f, -1)));
			target.m_normals[lod].emplace_back(glm::normalize(vec3(1, 0.5f, -1)));
			target.m_normals[lod].emplace_back(glm::normalize(vec3(-1, 0.5f, -1)));
			target.m_normals[lod].emplace_back(glm::normalize(vec3(1, 0.5f, 1)));

			target.m_uvs[lod].emplace_back(vec2(0, 0));
			target.m_uvs[lod].emplace_back(vec2(1, 0));
			target.m_uvs[lod].emplace_back(vec2(0, 1));
			target.m_uvs[lod].emplace_back(vec2(1, 1));

			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);
			target.m_extraData0[lod].emplace_back(extraData0);


			// Make faces 2 sided
			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 2);
			target.m_triangles[lod].emplace_back(index + 3);

			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 3);
			target.m_triangles[lod].emplace_back(index + 1);

			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 3);
			target.m_triangles[lod].emplace_back(index + 2);

			target.m_triangles[lod].emplace_back(index + 0);
			target.m_triangles[lod].emplace_back(index + 1);
			target.m_triangles[lod].emplace_back(index + 3);
		}
	}


};


ChunkJob_MeshTrees::ChunkJob_MeshTrees(Chunk* parent) : IChunkJob(parent)
{
}

void ChunkJob_MeshTrees::Execute() 
{
	const Chunk& chunk = GetOwningChunk();
	const PerlinNoise* noise = chunk.GetNoiseGenerator();

	// Attempt to generate trees on grass
	for (uint32 x = 0; x < CHUNK_SIZE; ++x)
		for (uint32 z = 0; z < CHUNK_SIZE; ++z)
			for (uint32 y = CHUNK_HEIGHT - 1; y > 0; --y)
			{
				ASSERT_JOB_VALID;

				Voxel::Type type = chunk.Get(x, y, z);
				if (type == Voxel::Type::Air)
					continue;

				// Attempt to generate tree on grass
				if (type == Voxel::Type::Grass)
				{
					const float v = noise->Get01(x * 0.9f, y * 0.9f, z * 0.9f);
					if (v <= 0.2f)
						AddTree(vec3(x, y, z));
				}

				// Don't continue, as found top voxel
				break;
			}
}

void ChunkJob_MeshTrees::OnComplete() 
{
	LeveledMesh* lodmesh = GetOwningChunk().GetTreeMesh();

	for (uint32 i = 0; i < TREE_LOD_COUNT; ++i)
	{
		Mesh* mesh = lodmesh->GetLoD(i);
		mesh->SetVertices(m_vertices[i]);
		mesh->SetNormals(m_normals[i]);
		mesh->SetUVs(m_uvs[i]);
		mesh->SetUVs(m_extraData0[i], 1);
		mesh->SetTriangles(m_triangles[i]);
	}

	GetOwningChunk().bIsTreeMeshBuilt = true;
}

void ChunkJob_MeshTrees::AddTree(const vec3& offset) 
{
	const Chunk& chunk = GetOwningChunk();
	const vec3 totalOffset = vec3(CHUNK_SIZE * chunk.GetCoords().x, 0, CHUNK_SIZE * chunk.GetCoords().y) + offset;
	const PerlinNoise* noise = chunk.GetNoiseGenerator();
	
	const float v = noise->Get01(totalOffset.x, totalOffset.y, totalOffset.z);
	TreeBranch tree(noise, totalOffset, vec2(0,0), 
		1.0f + v * 4.0f, // Start Length
		0.5f + v * 2.0f, // Start width
		0.95f,	// Length decay factor
		0.3f	// Width decay rate
	);


	for (uint32 i = 0; i < TREE_LOD_COUNT; ++i)
		tree.GenerateTreeMeshData(i, *this);
}