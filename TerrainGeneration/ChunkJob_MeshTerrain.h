#pragma once
#include "Common.h"
#include "IChunkJob.h"

#include <vector>
#include <unordered_map>



/**
* Job for building the mesh for the main terrain
*/
class ChunkJob_MeshTerrain : public IChunkJob
{
private:
	///
	/// Temp mesh vars
	///
	std::vector<uint32> m_triangles;
	std::vector<vec3> m_vertices;
	std::vector<vec3> m_normals;


public:
	ChunkJob_MeshTerrain(Chunk* parent);

	/** The main logic that this job should do (Should safely exit out if owning chunk has been destroyed) */
	virtual void Execute();

	/** Called once this job has been completed (Called from the main thread) */
	virtual void OnComplete();
};

