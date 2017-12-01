#include "Window.h"
#include "Logger.h"


#include "Mesh.h"
#include "Shader.h"

Mesh* testMesh;
Shader testShader;


void MainLoop(Window& window, const float& deltaTime)
{
	glClearColor(0.1451f, 0.1490f, 0.1922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	testShader.Bind();
	 
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

		layout (location = 0) in vec3 inPos;

		out vec3 pos;

		void main()
		{
			gl_Position = vec4(inPos, 1);
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
		vec3(-0.4f, 0.1f, 0.0f),
		vec3(0.4f, 0.1f, 0.0f),
		vec3(0.0f, 0.7f, 0.0f)
	}));
	mesh.SetTriangles(std::vector<uint32>({ 0,1,2 }));
	testMesh = &mesh;
	
	



	window.LaunchMainLoop(MainLoop);


	Window::DestroyAPI();
	return 0;
}