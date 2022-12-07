#include "imgui/imgui.h"
#include "App.h"
#include "Project_SkeletonAnimation.h"

Project_SkeletonAnimation::Project_SkeletonAnimation(App* _p_parent_app) :
	Project(_p_parent_app, "Project SkeletonAnimation"), active_model(0) {
}

void Project_SkeletonAnimation::Setup() {
	//Load the first model
	FBXLoader tad_fbx_loader("Tad.fbx");
	tad_fbx_loader.ExtractSceneData();
	Model* tad_model = new Model();
	tad_model->LoadModel(p_parent_app->GetWindow().Gfx(), 
						 &tad_fbx_loader, TEXT("Tad.png"));
	draw_models.emplace_back(tad_model);

	//Load the second model
	FBXLoader& fbx_ref = p_parent_app->GetSceneLoader();
	Model* human_model = new Model();
	human_model->LoadModel(p_parent_app->GetWindow().Gfx(),
		&fbx_ref, TEXT("Max_Red_Body_Diffuse.png"));
	draw_models.emplace_back(human_model);

	p_parent_app->GetCamera().SetR(15.0f);
}

void Project_SkeletonAnimation::Enter() {
	Camera& cam = p_parent_app->GetCamera();
	if (active_model == 0)
		cam.SetR(15.f);
	else
		cam.SetR(350.f);
	
} 

void Project_SkeletonAnimation::Update(float dt) {
	Window& window_ref = p_parent_app->GetWindow();
	
	ProjectControls();

	draw_models[active_model]->Update(window_ref.keyboard.isKeyPressed(VK_SPACE) ? 0.0f : dt);
}

void Project_SkeletonAnimation::Draw() {
	Window& window_ref = p_parent_app->GetWindow();
	draw_models[active_model]->Draw(window_ref.Gfx());
}

void Project_SkeletonAnimation::ProjectControls() {
	Camera& cam = p_parent_app->GetCamera();
	if (ImGui::BeginMenu("Active Model")) {
		if (ImGui::MenuItem("Tad", "", active_model == 0)) { 
			active_model = 0;
			cam.SetR(15.0f);
		}
		if (ImGui::MenuItem("Human", "", active_model == 1)) { 
			active_model = 1; 
			cam.SetR(350.0f);
		}
		ImGui::EndMenu();
	}
	unsigned int indx = 0;
	if (ImGui::BeginMenu("Active Animation")) {
		for (auto& animation : draw_models[active_model]->controller->animations) {
			std::string str = "Animation " + std::to_string(indx);
			bool selected = draw_models[active_model]->controller->active_animation == animation;
			if (ImGui::MenuItem(str.c_str(), "", selected)) {
				draw_models[active_model]->controller->SetActiveAnimation(indx);
			}
			indx++;
		}
		ImGui::EndMenu();
	}
}
