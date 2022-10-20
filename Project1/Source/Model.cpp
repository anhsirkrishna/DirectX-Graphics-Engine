#include "Model.h"

void Model::LoadModel(Graphics& gfx, FBXLoader* fbx_loader) {
	FBXMesh fbx_mesh = *(fbx_loader->meshes[0]);
	mesh = new Mesh(gfx, fbx_mesh);
	
	FBXSkeleton fbx_skele = fbx_loader->skele;
	controller = new AnimationController();
	Skeleton* skeleton = new Skeleton();
	skeleton->ConvertFromFbx(&fbx_skele);
	controller->SetSkel(skeleton);

	for (auto& animation : fbx_loader->animations) {
		controller->AddAnimation(new Animation());
		controller->active_animation->ConvertFromFbx(animation);
	}
	is_bind_pose = true;
}

void Model::Draw(Graphics& gfx) {
	
	//for (unsigned int i = 0; i < 60; ++i)
	for (unsigned int i = 0; i < controller->skeleton->hierarchy.size(); ++i)
	{
		//The matrices passed to the shader transform the vertex into bone space and then apply the bones animation
		mesh->bones_cbuf.bones_transform[i] = controller->skeleton->hierarchy[i]->inv_bind_transform.toMatrix() * controller->bone_matrix_buffer[i];
	}
	mesh->SyncBones(gfx);
	mesh->Draw(gfx);
}


void Model::Update(float dt) noexcept {
	controller->Update(dt);
	if (is_bind_pose)
		controller->ProcessBindPose();
	else
		controller->Process();
}