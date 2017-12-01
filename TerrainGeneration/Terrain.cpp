#include "Terrain.h"
#include "Logger.h"


#include <chrono>


#include "Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "Transform.h"
Mesh* testMesh;
Shader* testShader;
Material* testMaterial;



Terrain::Terrain(Scene* scene) :
	m_parent(scene)
{
	m_workerThread = new std::thread(&Terrain::RunWorker, this);


	testMesh = new Mesh;
	testMesh->SetVertices(std::vector<vec3>({
		vec3(0.0f, 5.0f, 0.0f),
		vec3(-3.0f, 0.0f, 3.0f),
		vec3(3.0f, 0.0f, 3.0f),

		vec3(0.0f, 5.0f, 0.0f),
		vec3(3.0f, 0.0f, -3.0f),
		vec3(-3.0f, 0.0f, -3.0f),

		vec3(0.0f, 5.0f, 0.0f),
		vec3(-3.0f, 0.0f, -3.0f),
		vec3(-3.0f, 0.0f, 3.0f),

		vec3(0.0f, 5.0f, 0.0f),
		vec3(3.0f, 0.0f, 3.0f),
		vec3(3.0f, 0.0f, -3.0f),
	}));
	testMesh->SetTriangles(std::vector<uint32>({ 0,1,2, 3,4,5, 6,7,8, 9,10,11 }));


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
			pos = inPos;
		}
	)");
	testShader->LoadFragmentShaderFromMemory(R"(
		#version 330 core

		in vec3 pos;

		out vec4 outColour;

		void main()
		{
			outColour.rgb = pos;
			outColour.a = 1;
		}
	)");
	testShader->LinkShader();

	testMaterial = new Material(testShader);
}

Terrain::~Terrain()
{
	if (m_workerThread != nullptr)
	{
		bWorkerRunning = false;
		m_workerThread->join();
		delete m_workerThread;
	}

	delete testMesh;

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

void Terrain::UpdateScene(Window& window, const float& deltaTime) 
{
}

void Terrain::RenderTerrain(Window& window, const float& deltaTime) 
{
	testMaterial->Bind(window, *m_parent);
	testMaterial->PrepareMesh(*testMesh);


	testMaterial->RenderInstance(Transform());

	Transform a;
	a.Translate(vec3(-6, 0, 0));
	a.SetEularRotation(vec3(45, 45, 0));
	testMaterial->RenderInstance(a);

	Transform b;
	b.Translate(vec3(6, 0, 0));
	testMaterial->RenderInstance(b);
}