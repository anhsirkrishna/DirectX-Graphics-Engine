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
	void LoadModel(Graphics& gfx, FBXLoader* fbx_loader);
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
};


