#include "Project_PathAnimation.h"

Project_PathAnimation::Project_PathAnimation(App* _p_parent_app) : 
	Project(_p_parent_app, "Path Animation") {

}

void Project_PathAnimation::Setup() {
	draw_model = std::make_unique<Model>();
	Graphics& gfx_ref = p_parent_app->GetWindow().Gfx();
	FBXLoader& fbx_ref = p_parent_app->GetSceneLoader();
	draw_model->LoadModel(p_parent_app->GetWindow().Gfx(), &fbx_ref, TEXT("Max_Red_Body_Diffuse.png"));

	std::unique_ptr<Path> animation_path = std::make_unique<Path>();
	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(-140.0f, 0.0f, 190.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-180.0f, 0.0f, 150.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-200.0f, 0.0f, 110.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-180.0f, 0.0f, 50.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-140.0f, 0.0f, 10.0f), 0);

	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(-100.0f, 0.0f, 50.0f), 1);
	animation_path->AddControlPoint(dx::XMFLOAT3(-20.0f, 0.0f, -90.0f), 1);
	animation_path->AddControlPoint(dx::XMFLOAT3(40.0f, 0.0f, -10.0f), 1);
	animation_path->AddControlPoint(dx::XMFLOAT3(100.0f, 0.0f, -90.0f), 1);

	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(140.0f, 0.0f, -110.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(110.0f, 0.0f, 10.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(160.0f, 0.0f, 30.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(110.0f, 0.0f, 70.0f), 2);
	animation_path->AddControlPoint(dx::XMFLOAT3(160.0f, 0.0f, 110.0f), 2);

	animation_path->AddControlSegment();
	animation_path->AddControlPoint(dx::XMFLOAT3(120.0f, 0.0f, 150.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(40.0f, 0.0f, 190.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(0.0f, 0.0f, 230.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-40.0f, 0.0f, 250.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-80.0f, 0.0f, 270.0f), 3);
	animation_path->AddControlPoint(dx::XMFLOAT3(-120.0f, 0.0f, 220.0f), 3);

	animation_path->Scale(2.0f, 2.0f, 2.0f);

	draw_path = std::make_unique<Curve>(gfx_ref, animation_path->GeneratePath());
	animation_path->GenerateForwardDiffTable();
	animation_path->GenerateDefaultVelocityFunction();

	draw_model->controller->SetAnimationPath(animation_path.release());
}

void Project_PathAnimation::Update(float dt) {
	Window& window_ref = p_parent_app->GetWindow();
	draw_model->Update(window_ref.keyboard.isKeyPressed(VK_SPACE) ? 0.0f : dt);
	draw_path->Update(dt);
}

void Project_PathAnimation::Draw() {
	Window& window_ref = p_parent_app->GetWindow();
	draw_model->Draw(window_ref.Gfx());
	draw_path->Draw(window_ref.Gfx());
}
