#pragma once
#include "Window.h"
#include "Camera.h"
#include "Drawable.h"
#include "TimerWrap.h"
#include "ImGUIManager.h"

class App {
public:
	App();

	int Run();
private:
	ImGUIManager imgui;
	Window window;

	void Update();
	
	std::vector<std::unique_ptr<class Drawable>> drawables;
	std::vector<class Box*> boxes;
	float speed_factor = 1.0f;
	Camera cam;

	TimerWrap timer;
	static constexpr size_t nDrawables = 2;
	void SpawnSimulationWindow() noexcept;
};

