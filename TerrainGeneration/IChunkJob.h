#pragma once


#define ASSERT_JOB_VALID if (IsAborted())return;


/**
* Interface for execute a task relating to a specific chunk
* (These jobs will be executed in a sperate thread)
*/
class IChunkJob
{
private:
	class Chunk& m_owner;
	bool bIsAborted = false;

public:
	IChunkJob(Chunk* owner) : m_owner(*owner) {}
	virtual ~IChunkJob() {}

	/** The main logic that this job should do (Should safely exit out if owning chunk has been destroyed) */
	virtual void Execute() = 0;

	/** Called once this job has been completed (Called from the main thread) */
	virtual void OnComplete() = 0;


	///
	/// Getters & Setters
	///
public:
	inline Chunk& GetOwningChunk() const { return m_owner; }

	inline bool IsAborted() const { return bIsAborted; }
	inline void Abort() { bIsAborted = true; }
};

