#include "Window.h"
#include "Logger.h"

#include "Scene.h"

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

	Scene scene;
	scene.Build();
	window.LaunchMainLoop(std::bind(&Scene::UpdateCallback, &scene, std::placeholders::_1, std::placeholders::_2));
	scene.Destroy();


	Window::DestroyAPI();
	return 0;
}