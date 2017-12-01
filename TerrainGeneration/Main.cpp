#include "Window.h"
#include "Logger.h"


#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

Camera testCamera;
Mesh* testMesh;
Shader testShader;


void MainLoop(Window& window, const float& deltaTime)
{
	glClearColor(0.1451f, 0.1490f, 0.1922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	testCamera.Translate(vec3(0, 0, -1) * deltaTime);

	testShader.SetCullFace(false);
	testShader.Bind();

	testShader.SetUniformMat4(testShader.GetUniform("ObjectToWorld"), mat4(1.0));
	testShader.SetUniformMat4(testShader.GetUniform("WorldToView"), testCamera.GetViewMatrix());
	testShader.SetUniformMat4(testShader.GetUniform("ViewToClip"), testCamera.GetPerspectiveMatrix(&window));
	 
	glBindVertexArray(testMesh->GetID());
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawElements(GL_TRIANGLES, testMesh->GetTriangleCount(), GL_UNSIGNED_INT, nullptr);
}


int main(void)
{
	if (!Window::InitAPI())
		return -1;

	Window window;

	WindowInit settings;
	settings.Major = 4;
	settings.Minor = 2;
	settings.Title = "Perlin Terrain";
	settings.bVerticalSync = true;

	if (!window.Open(settings)) 
	{
		LOG_ERROR("Failed to open window");
		return -1;
	}




	testShader.LoadVertexShaderFromMemory(R"(
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
	testShader.LoadFragmentShaderFromMemory(R"(
		#version 330 core

		in vec3 pos;

		out vec4 outColour;

		void main()
		{
			outColour.rgb = pos;
			outColour.a = 1;
		}
	)");
	testShader.LinkShader();


	// Setup whole triangle
	Mesh mesh;
	mesh.SetVertices(std::vector<vec3>({
		vec3(0.0f, 5.0f, 0.0f),
		vec3(-3.0f, 0.0f, 3.0f),
		vec3(3.0f, 5.0f, 3.0f),

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
	mesh.SetTriangles(std::vector<uint32>({ 0,1,2, 3,4,5, 6,7,8, 9,10,11 }));
	testMesh = &mesh;
	
	



	window.LaunchMainLoop(MainLoop);


	Window::DestroyAPI();
	return 0;
}