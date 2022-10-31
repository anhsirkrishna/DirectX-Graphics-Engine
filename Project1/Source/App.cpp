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

	class Factory
	{
	public:
		Factory(Graphics& gfx)
			:
			gfx(gfx)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			const DirectX::XMFLOAT4 mat = { cdist(rng),cdist(rng),cdist(rng), 1.0f };
			return std::make_unique<Box>(
				gfx, rng, adist, ddist,
				odist, rdist, bdist, mat
				);
			
		}
	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> sdist{ 0,4 };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_real_distribution<float> cdist{ 0.0f,1.0f };
		std::uniform_int_distribution<int> tdist{ 3,30 };
	};

	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ window.Gfx() });

	// init box pointers for editing instance parameters
	for (auto& pd : drawables)
	{
		if (auto pb = dynamic_cast<Box*>(pd.get()))
		{
			boxes.push_back(pb);
		}
	}

	window.Gfx().SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 1000.0f));
	DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	draw_line = std::make_unique<Line>(window.Gfx(), color);

	fbx_loader.ExtractSceneData();
	
	draw_model = std::make_unique<Model>();
	draw_model->LoadModel(window.Gfx(), &fbx_loader);

	animation_path = std::make_unique<Path>();
	animation_path->AddControlPoint(dx::XMFLOAT3(-40.0f, 0.0f, 40.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(-40.0f, 0.0f,  0.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(-40.0f, 0.0f, -40.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(0.0f, 0.0f, -40.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(40.0f, 0.0f, -40.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(40.0f, 0.0f, 0.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(40.0f, 0.0f, 40.0f));
	animation_path->AddControlPoint(dx::XMFLOAT3(0.0f, 0.0f, 40.0f));
	draw_path = std::make_unique<Curve>(window.Gfx(), animation_path->GeneratePath());

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
	for (auto& d : drawables)
	{
		d->Update(window.keyboard.isKeyPressed(VK_SPACE) ? 0.0f : dt);
		d->Draw(window.Gfx());
	}
	
	dx::XMVECTOR model_pos = animation_path->GetPosition(space_curve_u);
	draw_model->position.x = dx::XMVectorGetX(model_pos);
	draw_model->position.y = dx::XMVectorGetY(model_pos);
	draw_model->position.z = dx::XMVectorGetZ(model_pos);

	draw_model->Update(dt);
	draw_model->Draw(window.Gfx());

	draw_path->Update(dt);
	draw_path->Draw(window.Gfx());

	animation_path->ShowPathControls();

	SpawnSpaceCurveWindow();
	SpawnSimulationWindow();
	cam.SpawnCameraControls();
	window.Gfx().SwapBuffer();
}


void App::SpawnSimulationWindow() noexcept
{
	if (ImGui::Begin("Simulation Speed"))
	{
		ImGui::SliderFloat("Curve U", &space_curve_u, 0.0f, 1.0f);
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

void App::SpawnSpaceCurveWindow() noexcept
{
	if (ImGui::Begin("Space Curve"))
	{
		ImGui::SliderFloat("Curve U", &space_curve_u, 0.0f, 1.0f);
	}
	ImGui::End();
}