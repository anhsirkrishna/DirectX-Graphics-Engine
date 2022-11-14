#include "App.h"
#include "Box.h"
#include "MathWrap.h"
#include "FBXMatConverter.h"
#include <memory>
#include <algorithm>
#include <iterator>
#include "imgui/imgui.h"

int WindowHeight = 720;
int WindowWidth = 1280;

//int WindowHeight = 600;
//int WindowWidth = 800;

namespace dx = DirectX;

App::App() : window(WindowWidth, WindowHeight, TEXT("Direct3D Engine")) {
	window.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 1000.0f));
	DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	draw_line = std::make_unique<Line>(window.Gfx(), color);

	fbx_loader.ExtractSceneData();
	
	draw_model = std::make_unique<Model>();
	draw_model->LoadModel(window.Gfx(), &fbx_loader);

	std::unique_ptr<Path> animation_path = std::make_unique<Path>();
	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(-240.0f, 0.0f, 240.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-280.0f, 0.0f, 200.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-300.0f, 0.0f, 160.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-280.0f, 0.0f, 100.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-240.0f, 0.0f, 60.0f), 0);

	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(-200.0f, 0.0f, 100.0f), 1);
	animation_path->AddControlPoint(dx::XMFLOAT3(-120.0f, 0.0f, -40.0f), 1);
	animation_path->AddControlPoint(dx::XMFLOAT3(-60.0f, 0.0f, 40.0f), 1);
	animation_path->AddControlPoint(dx::XMFLOAT3(0.0f, 0.0f, -40.0f), 1);

	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(40.0f, 0.0f, -60.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(10.0f, 0.0f, 60.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(60.0f, 0.0f, 80.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(10.0f, 0.0f, 120.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(60.0f, 0.0f, 160.0f), 2);

	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(20.0f, 0.0f, 200.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-60.0f, 0.0f, 240.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-100.0f, 0.0f, 280.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-140.0f, 0.0f, 300.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-180.0f, 0.0f, 320.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-220.0f, 0.0f, 270.0f), 3);

	animation_path->Scale(2.0f, 2.0f, 2.0f);

	draw_path = std::make_unique<Curve>(window.Gfx(), animation_path->GeneratePath());
	animation_path->GenerateForwardDiffTable();
	animation_path->GenerateDefaultVelocityFunction();

	draw_model->controller->SetAnimationPath(animation_path.release());
}

int App::Run() {
	while (true) {
		if (const auto ecode = Window::ProcessMessages()) {
			return *ecode;
		}
		Update();
	}
}

void App::Update() {

	const auto dt = timer.Mark() * speed_factor;
	window.Gfx().BeginFrame();
	window.Gfx().SetCamera(cam.GetMatrix());

	draw_model->Update(window.keyboard.isKeyPressed(VK_SPACE) ? 0.0f : dt);
	draw_model->Draw(window.Gfx());

	draw_path->Update(dt);
	draw_path->Draw(window.Gfx());

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