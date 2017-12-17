#include "Scene.h"
#include "Logger.h"


void Scene::Build() 
{
	m_terrain = new Terrain(this, 1234);
	m_camera.SetLocation(vec3(0, CHUNK_HEIGHT, 0));

	m_camera.SetFarPlane(2000.0f);
	m_skyMesh = new Mesh;
	m_skyMaterial = new SkyMaterial;

	// Generate sky mesh
	m_skyMesh->SetVertices(std::vector<vec3>(
	{
		vec3(-1.0f, -1.0f, -1.0f),
		vec3(1.0f, -1.0f, -1.0f),
		vec3(-1.0f, -1.0f, 1.0f),
		vec3(1.0f, -1.0f, 1.0f),

		vec3(-1.0f, 1.0f, -1.0f),
		vec3(1.0f, 1.0f, -1.0f),
		vec3(-1.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f),
	}));
	m_skyMesh->SetTriangles(std::vector<uint32>(
	{
		0,2,1, 2,3,1,
		4,5,6, 5,7,6,

		2,6,3, 6,7,3,
		3,7,1, 1,7,5,
		1,4,0, 1,5,4,
		0,4,2, 2,4,6,

	}));
}

void Scene::Destroy() 
{
	delete m_terrain;
	delete m_skyMesh;
	delete m_skyMaterial;
}


void Scene::UpdateScene(Window& window, const float& deltaTime) 
{
	Keyboard& keyboard = window.GetKeyboard();
	Mouse& mouse = window.GetMouse();

	// Update camera movement
	{
		if (mouse.IsButtonReleased(Mouse::Button::MB_LEFT))
			mouse.SetGrabbed(!mouse.IsGrabbed());

		// Only moved if mouse is grabbed
		if (mouse.IsGrabbed())
		{
			const float baseMoveSpeed = 8.0f;
			float mouseSensitivity = 1.18f * 0.05f;

			if (keyboard.IsKeyDown(Keyboard::Key::KV_LSHIFT))
			{
				m_cameraCurrentSpeed += deltaTime * 10.0f;
				if (m_cameraCurrentSpeed > 50.0f)
					m_cameraCurrentSpeed = 50.0f;
			}
			else
				m_cameraCurrentSpeed = baseMoveSpeed;

			if (keyboard.IsKeyDown(Keyboard::Key::KV_W))
				m_camera.Translate(m_camera.GetForward2D() * deltaTime * m_cameraCurrentSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_S))
				m_camera.Translate(-m_camera.GetForward2D() * deltaTime * m_cameraCurrentSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_D))
				m_camera.Translate(m_camera.GetRight2D() * deltaTime * m_cameraCurrentSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_A))
				m_camera.Translate(-m_camera.GetRight2D() * deltaTime * m_cameraCurrentSpeed);

			if (keyboard.IsKeyDown(Keyboard::Key::KV_SPACE))
				m_camera.Translate(vec3(0, 1, 0) * deltaTime * m_cameraCurrentSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_LCONTROL))
				m_camera.Translate(vec3(0, -1, 0) * deltaTime * m_cameraCurrentSpeed);

			m_camera.SetEularRotation(m_camera.GetEularRotation() + vec3(-mouse.GetVelocity().y, -mouse.GetVelocity().x, 0) * mouseSensitivity);
		}
	}

	m_terrain->UpdateScene(window, deltaTime);
}

void Scene::RenderScene(Window& window, const float& deltaTime) 
{
	glClearColor(0.1451f, 0.1490f, 0.1922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Draw sky
	m_skyMaterial->Bind(window, *this);
	m_skyMaterial->PrepareMesh(*m_skyMesh);
	m_skyMaterial->RenderInstance(Transform());

	m_terrain->RenderTerrain(window, deltaTime);
	m_terrain->RenderTrees(window, deltaTime);
	m_terrain->RenderWater(window, deltaTime);
}