#include "SolidSphere.h"
#include "Project_IK.h"
#include "imgui/imgui.h"

Project_IK::Project_IK(App* _p_parent_app) :
	Project(_p_parent_app, "Inverse Kinematics"), target_position() {

}

void Project_IK::Setup() {
	draw_model = std::make_unique<Model>();
	Graphics& gfx_ref = p_parent_app->GetWindow().Gfx();
	FBXLoader& fbx_ref = p_parent_app->GetSceneLoader();
	draw_model->LoadModel(p_parent_app->GetWindow().Gfx(), &fbx_ref, TEXT("Max_Red_Body_Diffuse.png"));

	target_sphere = std::make_unique<SolidSphere>(gfx_ref, 10);
	target_position = dx::XMFLOAT3(-100.0f, 0.0f, -50.0f);

	dx::XMFLOAT3 starting_position = dx::XMFLOAT3(-480.0f, 0.0f, 380.0f);

	bool looped = false;
	std::unique_ptr<Path> animation_path = std::make_unique<Path>(looped);
	animation_path->AddControlSegment();
	animation_path->AddControlPoint(starting_position, 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-440.0f, 0.0f, 300.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-400.0f, 0.0f, 220.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-360.0f, 0.0f, 100.0f), 0);
	animation_path->AddControlPoint(dx::XMFLOAT3(-280.0f, 0.0f, 20.0f), 0);
	animation_path->AddControlPoint(target_position, 0);

	auto path_points = animation_path->GeneratePath();
	draw_path = std::make_unique<Curve>(gfx_ref, path_points);
	animation_path->GenerateForwardDiffTable();
	animation_path->GenerateDefaultVelocityFunction();

	animation_path->loop_time = 15;
	draw_model->controller->SetAnimationPath(animation_path.release());
}

void Project_IK::Update(float dt) {
	Window& window_ref = p_parent_app->GetWindow();
	
	ControlSpherePos();

	dx::XMFLOAT3 sphere_pos(target_position);
	sphere_pos.x += 30;
	sphere_pos.y += 105.0f;
	sphere_pos.z += 30;
	target_sphere->SetPosition(sphere_pos);

	float distance_to_target = dx::XMVectorGetX(
		dx::XMVector3Length(
			dx::XMVectorSubtract(
				dx::XMLoadFloat3(&draw_model->position), 
				dx::XMLoadFloat3(&target_position)))
	);
	if (distance_to_target < distance_threshold || draw_model->ik_mode) {
		draw_model->ik_mode = true;
		draw_model->ik_controller->target_position = dx::XMLoadFloat3(&sphere_pos);
	}

	draw_model->Update(window_ref.keyboard.isKeyPressed(VK_SPACE) ? 0.0f : dt);
	draw_path->Update(dt);
	target_sphere->Update(dt);
}

void Project_IK::Draw() {
	Window& window_ref = p_parent_app->GetWindow();
	draw_model->Draw(window_ref.Gfx());
	draw_path->Draw(window_ref.Gfx());
	target_sphere->Draw(window_ref.Gfx());
}

void Project_IK::ControlSpherePos() {
	if (ImGui::Begin("Sphere control"))
	{
		ImGui::SliderFloat("Sphere X pos", &target_position.x, -300.0f, 300.0f);
		ImGui::SliderFloat("Sphere Y pos", &target_position.y, -300.0f, 300.0f);
		ImGui::SliderFloat("Sphere Z pos", &target_position.z, -300.0f, 300.0f);
	}
	ImGui::End();
}
