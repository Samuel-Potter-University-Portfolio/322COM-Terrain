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

	LOG("Controls:");
	LOG("\tWS: \t\tForward/Back");
	LOG("\tAD: \t\tLeft/Right");
	LOG("\tSpace: \t\tUp");
	LOG("\tLCtrl: \t\tDown");
	LOG("\tMouse: \t\tLook");
	LOG("\tLMB: \t\tGrab mouse");
	LOG("\tLShift: \tSpeed up (Sprint)");

	Scene* scene = new Scene;
	scene->Build();
	window.LaunchMainLoop(std::bind(&Scene::UpdateCallback, scene, std::placeholders::_1, std::placeholders::_2));
	scene->Destroy();
	delete scene;

	Window::DestroyAPI();
	return 0;
}