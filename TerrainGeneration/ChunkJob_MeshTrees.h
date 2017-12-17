#pragma once
#include "Common.h"
#include "IChunkJob.h"

#include <vector>


#define TREE_LOD_COUNT 4



/**
* Job for building the mesh for the main terrain
*/
class ChunkJob_MeshTrees : public IChunkJob
{
private:
	friend struct TreeBranch;

	///
	/// Temp mesh vars
	///
	std::vector<uint32> m_triangles[TREE_LOD_COUNT];
	std::vector<vec3> m_vertices[TREE_LOD_COUNT];
	std::vector<vec3> m_normals[TREE_LOD_COUNT];
	std::vector<vec2> m_uvs[TREE_LOD_COUNT];
	std::vector<vec2> m_extraData0[TREE_LOD_COUNT]; // Encoded in uv channel 1 contains: texture type and sway weights


public:
	ChunkJob_MeshTrees(Chunk* parent);

	/** The main logic that this job should do (Should safely exit out if owning chunk has been destroyed) */
	virtual void Execute();

	/** Called once this job has been completed (Called from the main thread) */
	virtual void OnComplete();


private:
	/**
	* Add a tree to mesh data at this offset
	* @param offset			Offset from this 
	*/
	void AddTree(const vec3& offset);
};

