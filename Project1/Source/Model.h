#pragma once
#include "Animation.h"
#include "Mesh.h"
#include "FBXLoader.h"
#include "LineTree.h"

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

	Mesh* mesh;
	AnimationController* controller;
	LineTree* skeleton_drawable;

	bool is_bind_pose;
	bool draw_mesh;
	bool draw_skeleton;

	dx::XMFLOAT3 position;
	dx::XMFLOAT3 pitch;
	dx::XMFLOAT3 yaw;
	dx::XMFLOAT3 roll;
};


