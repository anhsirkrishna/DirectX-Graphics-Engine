#include "Model.h"
#include "imgui/imgui.h"

void Model::LoadModel(Graphics& gfx, FBXLoader* fbx_loader, const wchar_t* tex_file_path) {
	FBXMesh fbx_mesh = *(fbx_loader->meshes[0]);
	mesh = std::make_unique<Mesh>(gfx, fbx_mesh, tex_file_path);

	controller = std::make_unique<AnimationController>();

	controller->SetSkel(fbx_loader->skele);

	//The controller owns the animations 
	for (auto& animation : fbx_loader->animations) {
		controller->AddAnimation(*animation);
	}
	
	ik_controller = std::make_unique<IKController>();
	ik_controller->SetSkel(controller->GetSkelP());
	ik_controller->SetBaseAnimation(controller->animations[0]);

	int right_hand_index = 17;
	ik_controller->AddEndEffector(right_hand_index);
	ik_controller->GenerateDefaultRightArmConstraints(0);

	skeleton_drawable = std::make_unique<LineTree>(gfx, controller->GetSkel());

	is_bind_pose = false;
	draw_skeleton = true;
	draw_mesh = true;
}

void Model::Draw(Graphics& gfx) {
	SpawnModelControls();
	for (unsigned int i = 0; i < controller->skeleton->hierarchy.size(); ++i)
	{
		//The matrices passed to the shader transform the vertex into bone space and then apply the bones animation
		if (ik_mode) {
			mesh->bones_cbuf.bones_transform[i] = 
				ik_controller->p_skeleton->hierarchy[i]->inv_bind_transform.toMatrix() * 
				ik_controller->bone_matrix_buffer[i];
			skeleton_drawable->SetBoneTransform(i, ik_controller->bone_matrix_buffer[i]);
		}
		else {
			mesh->bones_cbuf.bones_transform[i] = 
				controller->skeleton->hierarchy[i]->inv_bind_transform.toMatrix() * 
				controller->bone_matrix_buffer[i];
			skeleton_drawable->SetBoneTransform(i, controller->bone_matrix_buffer[i]);
		}
		
	}

	if (draw_mesh) {
		mesh->SyncBones(gfx);
		mesh->Draw(gfx);
	}
	
	if (draw_skeleton) {
		gfx.DisableDepthTest();
		skeleton_drawable->SyncBones(gfx);
		skeleton_drawable->Draw(gfx);
		gfx.EnableDepthTest();
	}
}


void Model::Update(float dt) noexcept {
	if (not ik_mode) {
		//Get current position based on the animation path
		dx::XMVECTOR model_pos = controller->animation_path->GetCurrentPosition();
		position.x = dx::XMVectorGetX(model_pos);
		position.y = dx::XMVectorGetY(model_pos);
		position.z = dx::XMVectorGetZ(model_pos);

		//Get rotation matrix based on COI approach 
		dx::XMVECTOR look_pos = controller->animation_path->GetLookPosition();
		dx::XMVECTOR roll = dx::XMVectorSubtract(model_pos, look_pos);
		dx::XMVECTOR pitch = dx::XMVector3Cross(dx::XMVectorSet(0, 1, 0, 0), roll);
		dx::XMVECTOR yaw = dx::XMVector3Cross(roll, pitch);
		roll = dx::XMVector3Normalize(roll);
		pitch = dx::XMVector3Normalize(pitch);
		yaw = dx::XMVector3Normalize(yaw);

		rotation = dx::XMMatrixSet(
			pitch.m128_f32[0], pitch.m128_f32[1], pitch.m128_f32[2], 0,
			yaw.m128_f32[0], yaw.m128_f32[1], yaw.m128_f32[2], 0,
			roll.m128_f32[0], roll.m128_f32[1], roll.m128_f32[2], 0,
			0, 0, 0, 1
		);
	}

	mesh->SetRotation(rotation);
	skeleton_drawable->SetRotation(rotation);

	mesh->SetPosition(position);
	skeleton_drawable->SetPosition(position);

	skeleton_drawable->Update(dt);
	mesh->Update(dt);

	controller->Update(dt);
	
	if (is_bind_pose)
		controller->ProcessBindPose();
	else
		controller->Process();

	if (ik_mode) {
		ik_controller->Update(dt, dx::XMLoadFloat3(&position), rotation);
		ik_controller->Process(dt);
	}
}

void Model::Reset() {
	position = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotation = dx::XMMatrixIdentity();
}

void Model::SpawnModelControls() noexcept {
	if (ImGui::Begin("Model"))
	{
		ImGui::Checkbox("Display Mesh", &draw_mesh);
		ImGui::Checkbox("Display Skeleton", &draw_skeleton);
		ImGui::Checkbox("Bind pose", &is_bind_pose);
	}
	ImGui::End();
}