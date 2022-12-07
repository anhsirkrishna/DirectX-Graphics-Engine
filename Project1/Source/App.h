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
#include "Mesh.h"
#include "Model.h"
#include "Path.h"
#include "Curve.h"
#include "SolidSphere.h"
#include "DrawPlane.h"
#include "Project.h"

class App {
public:
	App();

	int Run();
	Window& GetWindow();
	FBXLoader& GetSceneLoader();
	Camera& GetCamera();
private:
	ImGUIManager imgui;
	Window window;
	FBXLoader fbx_loader;
	void Update();

	std::vector<std::unique_ptr<Project>> project_list;
	Project* active_project;

	float speed_factor = 1.0f;
	Camera cam;

	TimerWrap timer;
	static constexpr size_t nDrawables = 2;
	void SpawnSimulationWindow() noexcept;
	void ProjectSelectionMenu() noexcept;
};

