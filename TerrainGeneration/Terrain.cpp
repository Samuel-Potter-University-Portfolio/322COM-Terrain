#include "Terrain.h"
#include "Logger.h"
#include "Scene.h"

#include <chrono>
#include "Transform.h"



Terrain::Terrain(Scene* scene, const uint32& seed) :
	m_parent(scene),
	m_noiseGenerator(seed)
{
#ifdef _DEBUG
	m_workRadius = 1;
	m_loadRadius = 2;
	m_unloadRadius = 7;
#else
	m_workRadius = 16;
	m_loadRadius = 18;
	m_unloadRadius = 24;
#endif

	// Base pool size on unload range
	m_poolSize = (m_unloadRadius * 2 + 1)*(m_unloadRadius * 2 + 1) + 40;

	LOG("Using seed: %i", seed);
	LOG("Chunk Settings:");
	LOG("\t-WORK_RADIUS:\t%i", m_workRadius);
	LOG("\t-LOAD_RADIUS:\t%i", m_loadRadius);
	LOG("\t-UNLOAD_RADIUS:\t%i", m_unloadRadius);
	m_activeChunks.reserve(m_poolSize);


	// Setup chunk pool
	for (uint32 i = 0; i < m_poolSize; ++i)
		m_chunkPool.emplace(new Chunk(this));
	LOG("Built chunk pool of size %i", m_poolSize);
	

	// Load initialize small radius of chunkss
	for (int32 x = -2; x <= 2; ++x)
		for (int32 y = -2; y <= 2; ++y)
		{
			ivec2 coords(x, y);
			if (m_activeChunks.find(coords) == m_activeChunks.end())
			{
				Chunk* chunk;
				if (TryGetNewChunk(chunk, coords))
					m_activeChunks[coords] = chunk;
				else
					LOG_WARNING("Cannot load new chunk (%i,%i) as pool is empty", coords.x, coords.y);
			}
		}

	bWaitingOnFirstBuild = true;
	m_workerThread = new std::thread(&Terrain::RunWorker, this);
}

Terrain::~Terrain()
{
	// Clean up worker thread
	if (m_workerThread != nullptr)
	{
		LOG("Waiting for worker to finish..");
		bWorkerRunning = false;
		m_workerThread->join();
		delete m_workerThread;
	}

	// Clean up chunk pool
	for (auto it : m_activeChunks)
		delete it.second;
	while (m_chunkPool.size() != 0)
	{
		Chunk* chunk = m_chunkPool.front();
		m_chunkPool.pop();
		delete chunk;
	}
}

void Terrain::RunWorker() 
{
	bWorkerRunning = true;
	LOG("Launching Terrain worker thread");


	while (bWorkerRunning)
	{
		const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		try
		{
			ivec2 centre = m_loadCentre;
			

			// Prioritse jobs from centre in a spiral outwards
			// https://stackoverflow.com/questions/398299/looping-in-a-spiral
			int32 x = 0;
			int32 y = 0;
			int32 dx = 0;
			int32 dy = -1;

			const int32 jobRange = (centre != m_loadCentre) ? 2 : m_workRadius * 2 + 1; // Do small range for when centre changes
			const uint32 max = jobRange*jobRange;

			// Loop in spiral
			for (int32 i = 0; i < max; ++i)
			{
				// Don't continue with jobs, as we've finished
				if (!bWorkerRunning)
					break;


				// Reset loop if centre has moved
				if (centre != m_loadCentre)
				{
					i = 0;
					x = 0;
					y = 0;
					dx = 0;
					dy = -1;
					centre = m_loadCentre;
				}

				// Execute iteration
				if ((-jobRange / 2 < x) && (x <= jobRange / 2) && (-jobRange / 2 < y) && (y <= jobRange / 2))
				{
					// Make sure search is done safely (Only lock mutex while iterating)
					Chunk* chunk = nullptr;
					if (m_chunkAccessMutex.try_lock())
					{
						auto it = m_activeChunks.find(m_loadCentre + ivec2(x, y));

						if (it != m_activeChunks.end() && it->second->HasQueuedJob())
							chunk = it->second;

						m_chunkAccessMutex.unlock();
					}

					// Look for jobs to execute
					if (chunk != nullptr)
					{
						while (chunk->HasQueuedJob())
						{
							IChunkJob* job = chunk->GetQueuedJob();

							// Only execute job, if it hasn't been aborted
							if (!job->IsAborted())
								job->Execute();

							// Job may have aborted during execution
							if (!job->IsAborted())
								m_completedJobQueue.emplace(job);
							else
								delete job;
						}
					}
				}

				// Spiral logic
				if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1 - y)))
				{
					int32 temp = dx;
					dx = -dy;
					dy = temp;
				}
				x += dx;
				y += dy;
			}


			if (bWaitingOnFirstBuild)
			{
				m_previousCentre = vec2(-11341, 134785); // Force resync
				bWaitingOnFirstBuild = false;
			}
		}
		catch (std::exception e)
		{
			LOG_ERROR("Exception caught in worker thread '%s'", e.what());
			throw e;
		}


		const std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		const auto diff = std::chrono::time_point_cast<std::chrono::milliseconds>(end) - std::chrono::time_point_cast<std::chrono::milliseconds>(start);
		const auto sleepTime = std::chrono::milliseconds(1000 / 60);

		// Update at 60 ticks a second
		if (diff < sleepTime)
			std::this_thread::sleep_for(sleepTime - diff);
	}

	LOG("Terrain worker thread closed");
	bWorkerRunning = false;
}


bool Terrain::TryGetNewChunk(Chunk*& outChunk, const ivec2& coord)
{
	if (m_chunkPool.size() == 0)
		return false;

	// Take out of chunk pool
	outChunk = m_chunkPool.front();
	m_chunkPool.pop();

	outChunk->Alloc(coord);
	return true;
}
void Terrain::FreeChunk(Chunk* chunk) 
{
	// Add back into chunk pool
	m_chunkPool.push(chunk);
	chunk->Dealloc();
}


void Terrain::UpdateScene(Window& window, const float& deltaTime) 
{
	vec3 loadCentre = m_parent->GetCamera().GetLocation();
	ivec2 centre = GetChunkCoords(std::round(loadCentre.x), std::round(loadCentre.y), std::round(loadCentre.z));


	// Print centre information
	if (window.GetKeyboard().IsKeyReleased(Keyboard::Key::KV_C))
	{
		LOG("Load centre (%i, %i)", centre.x, centre.y);
		LOG("Loaded chunks (%i)", m_activeChunks.size());
	}


	// Only update if centre has changed
	if (centre != m_previousCentre && !bWaitingOnFirstBuild)
	{
		const std::lock_guard<std::mutex> guard(m_chunkAccessMutex);


		// Unload any chunks which are outside of the area
		for (auto it = m_activeChunks.begin(); it != m_activeChunks.end();)
		{
			const ivec2& coords = it->first;
			if (std::abs(coords.x - centre.x) > m_unloadRadius || std::abs(coords.y - centre.y) > m_unloadRadius)
			{
				// TODO - Cleanup any active jobs
				FreeChunk(it->second);
				m_activeChunks.erase(it++);
			}
			else
				++it;
		}


		// Load any chunks which are in load area and aren't already loaded
		for (int32 x = -m_loadRadius; x <= m_loadRadius; ++x)
			for (int32 y = -m_loadRadius; y <= m_loadRadius; ++y)
			{
				ivec2 coords(centre.x + x, centre.y + y);
				if (m_activeChunks.find(coords) == m_activeChunks.end())
				{
					Chunk* chunk;
					if (TryGetNewChunk(chunk, coords))
						m_activeChunks[coords] = chunk;
					else
						LOG_WARNING("Cannot load new chunk (%i,%i) as pool is empty", coords.x, coords.y);
				}
			}

		// Update centre after all chunks are in
		m_loadCentre = centre;
		m_previousCentre = centre;
	}

	

	// Complete any jobs that need it (Only complete max of 10 per update)
	for (uint32 i = 0; i < 10 && m_completedJobQueue.size() != 0; ++i)
	{
		IChunkJob* job = m_completedJobQueue.front();
		m_completedJobQueue.pop();

		// Only complete if not aborted
		if (!job->IsAborted())
		{
			job->OnComplete();
			if (!job->IsAborted())
				job->GetOwningChunk().OnJobCompletion(job);
		}

		delete job;
	}
}

void Terrain::RenderTerrain(Window& window, const float& deltaTime) 
{
	m_terrainMaterial.Bind(window, *m_parent);


	for (auto it : m_activeChunks)
	{
		if (it.second->IsTerrainMeshBuilt())
		{
			m_terrainMaterial.PrepareMesh(*it.second->GetTerrainMesh());
			m_terrainMaterial.RenderInstance(Transform());
		}
	}
}

void Terrain::RenderTrees(Window& window, const float& deltaTime) 
{
	m_treeMaterial.Bind(window, *m_parent);
	const vec3 cameraLocation = m_parent->GetCamera().GetLocation();

	for (auto it : m_activeChunks)
	{
		if (it.second->IsTreeMeshBuilt())
		{
			const vec3 chunkOffset = vec3(CHUNK_SIZE * it.second->GetCoords().x, 0, CHUNK_SIZE * it.second->GetCoords().y);
			const vec3 diff = chunkOffset - cameraLocation;

			m_terrainMaterial.PrepareMesh(*it.second->GetTreeMesh()->GetMesh(glm::dot(diff, diff)));
			m_terrainMaterial.RenderInstance(Transform());
		}
	}
}

void Terrain::RenderWater(Window& window, const float& deltaTime)
{
	m_waterMaterial.Bind(window, *m_parent);


	for (auto it : m_activeChunks)
	{
		if (it.second->IsWaterMeshBuilt())
		{
			m_terrainMaterial.PrepareMesh(*it.second->GetWaterMesh());
			m_terrainMaterial.RenderInstance(Transform());
		}
	}
}


Voxel::Type Terrain::Get(const int32& x, const int32& y, const int32& z) const 
{
	ivec2 coords = GetChunkCoords(x, y, z);

	auto it = m_activeChunks.find(coords);
	if (it == m_activeChunks.end())
		return Voxel::Type::Air;
	else
		return it->second->Get(x - coords.x * CHUNK_SIZE, y, z - coords.y * CHUNK_SIZE);
}

void Terrain::NotifyChunkGeneration(const ivec2& coords) 
{
	// Notify adjacent chunks of the change
	for (int32 x = -1; x <= 1; ++x)
		for (int32 y = -1; y <= 1; ++y)
		{
			if (x == 0 && y == 0)
				continue;

			auto it = m_activeChunks.find(ivec2(coords.x + x, coords.y + y));
			if (it != m_activeChunks.end())
				it->second->OnAdjacentChunkGenerate();
		}
}