#include "App.h"
#include "Box.h"
#include "MathWrap.h"
#include "FBXMatConverter.h"
#include <memory>
#include <algorithm>
#include <iterator>
#include "imgui/imgui.h"
#include "Project_PathAnimation.h"
#include "Project_IK.h"
#include "Project_SkeletonAnimation.h"
#include "Project_Physics.h"

int WindowHeight = 720;
int WindowWidth = 1280;

//int WindowHeight = 600;
//int WindowWidth = 800;

namespace dx = DirectX;

App::App() : window(WindowWidth, WindowHeight, TEXT("Direct3D Engine")) {
	cam.SetControlWindow(&window);

	window.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 2000.0f));

	fbx_loader.ExtractSceneData();

	project_list.emplace_back(new Project_SkeletonAnimation(this));
	project_list.emplace_back(new Project_PathAnimation(this));
	project_list.emplace_back(new Project_IK(this));
	project_list.emplace_back(new Project_Physics(this));
	
	active_project = project_list[3].get();
	
	for (auto& proj : project_list)
		proj->Setup();

	active_project->Enter();
	timer.Reset();
}

int App::Run() {
	while (true) {
		if (const auto ecode = Window::ProcessMessages()) {
			return *ecode;
		}
		Update();
	}
}

Window& App::GetWindow() {
	return window;
}

FBXLoader& App::GetSceneLoader() {
	return fbx_loader;
}

Camera& App::GetCamera() {
	return cam;
}

void App::Update() {
	const auto dt = timer.Mark() * speed_factor;
	window.Gfx().BeginFrame();
	
	cam.Update(dt);
	window.Gfx().SetCamera(cam.GetMatrix());

	ProjectSelectionMenu();

	active_project->Update(dt);
	active_project->Draw();

	SpawnSimulationWindow();
	cam.SpawnCameraControls();
	window.Gfx().SwapBuffer();
}


void App::SpawnSimulationWindow() noexcept
{
	if (ImGui::Begin("Simulation Speed"))
	{
		ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 6.0f, "%.4f", 3.2f);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", window.keyboard.isKeyPressed(VK_SPACE) ? "PAUSED" : "RUNNING (hold spacebar to pause)");
		if (ImGui::Button("Wireframe mode"))
			window.Gfx().SetWireframeMode();

		if (ImGui::Button("Fill mode"))
			window.Gfx().SetFillMode();
	}
	ImGui::End();
}

/*
* IMGUI menu to select which project to run
*/
void App::ProjectSelectionMenu() noexcept {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Project Selector")) {
			for (auto& proj : project_list) {
				if (ImGui::MenuItem(proj->GetName().c_str(), "", active_project == proj.get())) 
				{ 
					active_project = proj.get();
					active_project->Enter();
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
