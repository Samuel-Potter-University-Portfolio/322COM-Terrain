#pragma once
#include "Common.h"
#include "IChunkJob.h"


/**
* Job for generating the specific voxels of a chunk
*/
class ChunkJob_Generate : public IChunkJob
{
public:
	ChunkJob_Generate(Chunk* parent);

	/** The main logic that this job should do (Should safely exit out if owning chunk has been destroyed) */
	virtual void Execute();

	/** Called once this job has been completed (Called from the main thread) */
	virtual void OnComplete();
};

