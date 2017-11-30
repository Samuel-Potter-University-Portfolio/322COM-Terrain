#include "Window.h"
#include "Logger.h"


#include "Shader.h"
uint32 testVAO;
Shader testShader;

void MainLoop(Window& window, const float& deltaTime)
{
	glClearColor(0.1451f, 0.1490f, 0.1922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	testShader.SetCullFace(false);
	testShader.Bind();
	glDrawArrays(GL_TRIANGLES, testVAO, 3);
	//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
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
	uint32 testVBO[2];
	glGenVertexArrays(1, &testVAO);
	glGenBuffers(2, testVBO);


	glBindVertexArray(testVAO);
	float coords[]{
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, testVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), coords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);



	window.LaunchMainLoop(MainLoop);


	Window::DestroyAPI();


	/*
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	*/
	return 0;
}