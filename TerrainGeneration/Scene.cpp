#include "Scene.h"
#include "Logger.h"


void Scene::Build() 
{
	m_terrain = new Terrain(this);
	m_camera.SetLocation(vec3(0, 10, 0));
}

void Scene::Destroy() 
{
	delete m_terrain;
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
			const float baseMoveSpeed = 5.0f;
			float mouseSensitivity = 1.18f * 0.05f;

			if (keyboard.IsKeyDown(Keyboard::Key::KV_LSHIFT))
			{
				m_cameraCurrentSpeed += deltaTime * 5.0f;
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

	m_terrain->RenderTerrain(window, deltaTime);
}