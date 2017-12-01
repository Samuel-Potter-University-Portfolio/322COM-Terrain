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
		layout (location = 1) in vec3 inNormal;

		out vec3 passToCamera;
		out vec3 passPos;
		out vec3 passNormal;

		void main()
		{
			vec4 worldLocation = ObjectToWorld * vec4(inPos, 1);
			vec3 cameraLocation = vec3(ViewToClip[0][3], ViewToClip[1][3], ViewToClip[2][3]);
			passToCamera = worldLocation.xyz - cameraLocation;

			gl_Position = ViewToClip * WorldToView * worldLocation;
			passPos = inPos.xyz;
			passNormal = inNormal;
		}
	)");
	testShader->LoadFragmentShaderFromMemory(R"(
		#version 330 core

				
		in vec3 passToCamera;
		in vec3 passPos;
		in vec3 passNormal;

		out vec4 outColour;

		void main()
		{
			vec3 lightDirection = normalize(vec3(1, -1, 1));
			float diffuse = max(dot(-lightDirection, normalize(passNormal)), 0.2);

			outColour.rgb = vec3(1,1,1) * diffuse;
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
	for (int32 x = -1; x <= 1; ++x)
		for (int32 y = -1; y <= 1; ++y)
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
		if (it.second->IsMeshBuilt())
		{
			testMaterial->PrepareMesh(*it.second->GetMesh());
			testMaterial->RenderInstance(Transform());
		}
	}
}