#pragma once
#include "Window.h"
#include "Camera.h"
#include "Drawable.h"
#include "TimerWrap.h"
#include "ImGUIManager.h"
#include "FBXLoader.h"
#include "FBXSkeleton.h"
#include "Animation.h"
#include "Line.h"

class App {
public:
	App();

	int Run();
private:
	ImGUIManager imgui;
	Window window;
	FBXLoader fbx_loader;
	void Update();
	
	std::vector<std::unique_ptr<class Drawable>> drawables;
	std::vector<class Box*> boxes;
	std::unique_ptr<Line> draw_line;
	std::unique_ptr<Skeleton> draw_skeleton;
	float speed_factor = 1.0f;
	Camera cam;

	TimerWrap timer;
	static constexpr size_t nDrawables = 2;
	void SpawnSimulationWindow() noexcept;
};

