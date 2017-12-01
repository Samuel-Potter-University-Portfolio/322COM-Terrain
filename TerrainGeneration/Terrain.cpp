#include "Terrain.h"
#include "Logger.h"


#include <chrono>


#include "Shader.h"
#include "Material.h"
#include "Transform.h"

Shader* testShader;
Material* testMaterial;



Terrain::Terrain(Scene* scene) :
	m_parent(scene)
{
	m_workerThread = new std::thread(&Terrain::RunWorker, this);

	// Setup chunk pool
	for (uint32 i = 0; i < m_poolSize; ++i)
		m_chunkPool.emplace(new Chunk(this));
	LOG("Built chunk pool of size %i", m_poolSize);



	testShader = new Shader;
	testShader->LoadVertexShaderFromMemory(R"(
		#version 420 core

		// Transform matrix
		uniform mat4 ObjectToWorld;

		// View matrix
		uniform mat4 WorldToView;

		// Perspective matrix
		uniform mat4 ViewToClip;

		layout (location = 0) in vec3 inPos;

		out vec3 pos;

		void main()
		{
			gl_Position = ViewToClip * WorldToView * ObjectToWorld * vec4(inPos, 1);
			pos = inPos.xyz;
		}
	)");
	testShader->LoadFragmentShaderFromMemory(R"(
		#version 330 core

		in vec3 pos;

		out vec4 outColour;

		void main()
		{
			outColour.rgb = mod(pos, vec3(1,1,1));
			outColour.a = 1;
		}
	)");
	testShader->LinkShader();

	testMaterial = new Material(testShader);
}

Terrain::~Terrain()
{
	// Clean up worker thread
	if (m_workerThread != nullptr)
	{
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



	delete testMaterial;
	delete testShader;
}

void Terrain::RunWorker() 
{
	bWorkerRunning = true;
	LOG("Launching Terrain worker thread");

	while (bWorkerRunning)
	{
		// Update at 60 ticks a second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
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

	m_activeChunks[coord] = outChunk;
	outChunk->Alloc(coord);
	return true;
}
void Terrain::FreeChunk(Chunk* chunk) 
{
	// Remove from active chunks
	m_activeChunks.erase(chunk->GetCoords());

	// Add back into chunk pool
	m_chunkPool.push(chunk);
	chunk->Dealloc();
}


void Terrain::UpdateScene(Window& window, const float& deltaTime) 
{
	for (int32 x = -2; x <= 2; ++x)
		for (int32 y = -2; y <= 2; ++y)
		{
			ivec2 coords(x, y);
			if (m_activeChunks.find(coords) == m_activeChunks.end())
			{
				Chunk* chunk;
				TryGetNewChunk(chunk, coords);
			}
		}
}

void Terrain::RenderTerrain(Window& window, const float& deltaTime) 
{
	testMaterial->Bind(window, *m_parent);

	for (auto it : m_activeChunks)
	{
		testMaterial->PrepareMesh(*it.second->GetMesh());
		testMaterial->RenderInstance(Transform());
	}
}