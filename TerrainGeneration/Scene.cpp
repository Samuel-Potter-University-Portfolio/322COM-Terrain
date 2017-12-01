#include "Scene.h"


#include "Mesh.h"
#include "Shader.h"

Mesh* testMesh;
Shader* testShader;

void Scene::Build() 
{
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
}

void Scene::Destroy() 
{
	delete testShader;
	delete testMesh;
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
			float moveSpeed = 2.0f;
			float mouseSensitivity = 1.18f * 0.05f;

			if (keyboard.IsKeyDown(Keyboard::Key::KV_W))
				m_camera.Translate(m_camera.GetForward2D() * deltaTime * moveSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_S))
				m_camera.Translate(-m_camera.GetForward2D() * deltaTime * moveSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_D))
				m_camera.Translate(m_camera.GetRight2D() * deltaTime * moveSpeed);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_A))
				m_camera.Translate(-m_camera.GetRight2D() * deltaTime * moveSpeed);

			if (keyboard.IsKeyDown(Keyboard::Key::KV_SPACE))
				m_camera.Translate(vec3(0, 1, 0) * deltaTime);
			if (keyboard.IsKeyDown(Keyboard::Key::KV_LCONTROL))
				m_camera.Translate(vec3(0, -1, 0) * deltaTime);

			m_camera.SetEularRotation(m_camera.GetEularRotation() + vec3(-mouse.GetVelocity().y, -mouse.GetVelocity().x, 0) * mouseSensitivity);
		}
	}
}

void Scene::RenderScene(Window& window, const float& deltaTime) 
{
	glClearColor(0.1451f, 0.1490f, 0.1922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	testShader->Bind();
	testShader->SetUniformMat4(testShader->GetUniform("WorldToView"), m_camera.GetViewMatrix());
	testShader->SetUniformMat4(testShader->GetUniform("ViewToClip"), m_camera.GetPerspectiveMatrix(&window));

	glBindVertexArray(testMesh->GetID());
	testShader->SetUniformMat4(testShader->GetUniform("ObjectToWorld"), mat4(1.0));
	glDrawElements(GL_TRIANGLES, testMesh->GetTriangleCount(), GL_UNSIGNED_INT, nullptr);

	Transform testTransform;
	testTransform.SetLocation(vec3(-6, 0, 0));
	testShader->SetUniformMat4(testShader->GetUniform("ObjectToWorld"), testTransform.GetTransformMatrix());
	glDrawElements(GL_TRIANGLES, testMesh->GetTriangleCount(), GL_UNSIGNED_INT, nullptr);

	Transform testTransform1;
	testTransform1.SetLocation(vec3(6, 0, 0));
	testShader->SetUniformMat4(testShader->GetUniform("ObjectToWorld"), testTransform1.GetTransformMatrix());
	glDrawElements(GL_TRIANGLES, testMesh->GetTriangleCount(), GL_UNSIGNED_INT, nullptr);
}