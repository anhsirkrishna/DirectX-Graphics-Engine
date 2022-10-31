#include "Model.h"
#include "imgui/imgui.h"

void Model::LoadModel(Graphics& gfx, FBXLoader* fbx_loader) {
	FBXMesh fbx_mesh = *(fbx_loader->meshes[0]);
	mesh = new Mesh(gfx, fbx_mesh);
	
	FBXSkeleton fbx_skele = fbx_loader->skele;
	controller = new AnimationController();
	Skeleton* skeleton = new Skeleton();
	skeleton->ConvertFromFbx(&fbx_skele);
	skeleton->Initialize();
	controller->SetSkel(skeleton);

	for (auto& animation : fbx_loader->animations) {
		controller->AddAnimation(new Animation());
		controller->active_animation->ConvertFromFbx(animation);
	}
	skeleton_drawable = new LineTree(gfx, *skeleton);

	is_bind_pose = true;
	draw_skeleton = true;
	draw_mesh = true;
}

void Model::Draw(Graphics& gfx) {
	SpawnModelControls();
	//for (unsigned int i = 0; i < 60; ++i)
	for (unsigned int i = 0; i < controller->skeleton->hierarchy.size(); ++i)
	{
		//The matrices passed to the shader transform the vertex into bone space and then apply the bones animation
		mesh->bones_cbuf.bones_transform[i] = controller->skeleton->hierarchy[i]->inv_bind_transform.toMatrix() * controller->bone_matrix_buffer[i];
		skeleton_drawable->SetBoneTransform(i, controller->bone_matrix_buffer[i]);
	}
	if (draw_mesh) {
		mesh->SyncBones(gfx);
		mesh->Draw(gfx);
	}
	
	if (draw_skeleton) {
		skeleton_drawable->SyncBones(gfx);
		skeleton_drawable->Draw(gfx);
	}
}


void Model::Update(float dt) noexcept {
	
	skeleton_drawable->SetPosition(position);
	mesh->SetPosition(position);
	skeleton_drawable->Update(dt);
	mesh->Update(dt);
	controller->Update(dt);
	if (is_bind_pose)
		controller->ProcessBindPose();
	else
		controller->Process();
}

void Model::Reset() {
	position = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void Model::SpawnModelControls() noexcept {
	if (ImGui::Begin("Model"))
	{
		ImGui::Text("Model");
		ImGui::Checkbox("Display Mesh", &draw_mesh);
		ImGui::Checkbox("Display Skeleton", &draw_skeleton);
		ImGui::Checkbox("Bind pose", &is_bind_pose);
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &position.x, -1000.0f, 1000.0f, "%.1f");
		ImGui::SliderFloat("Y", &position.y, -1000.0f, 1000.0f, "%.1f");
		ImGui::SliderFloat("Z", &position.z, -1000.0f, 1000.0f, "%.1f");
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}