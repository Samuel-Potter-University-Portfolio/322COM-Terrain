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
}

void ChunkJob_MeshTrees::AddTree(const vec3& offset) 
{
	
}