#pragma once
#include "Animation.h"
#include "Mesh.h"
#include "FBXLoader.h"
#include "LineTree.h"
#include "IKinematics.h"

class Model
{
public:
	Model() = default;
	~Model() = default;
	void LoadModel(Graphics& gfx, FBXLoader* fbx_loader, const wchar_t* tex_file_path);
	void Draw(Graphics& gfx);
	void Update(float dt) noexcept;
	void Reset();

	void SpawnModelControls() noexcept;

	std::unique_ptr<Mesh> mesh;
	std::unique_ptr<LineTree> skeleton_drawable;
	
	std::unique_ptr<AnimationController> controller;
	std::unique_ptr<IKController> ik_controller;

	bool is_bind_pose;
	bool draw_mesh;
	bool draw_skeleton;

	//Get animations from IK Controller instead if this is true
	bool ik_mode;

	dx::XMFLOAT3 position;
	dx::XMMATRIX rotation;
	dx::XMFLOAT3 pitch;
	dx::XMFLOAT3 yaw;
	dx::XMFLOAT3 roll;
};


