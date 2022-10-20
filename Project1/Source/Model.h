#pragma once
#include "Mesh.h"
#include "Animation.h"
#include "FBXLoader.h"

class Model
{
public:
	Model() = default;
	~Model() = default;
	void LoadModel(Graphics& gfx, FBXLoader* fbx_loader);
	void Draw(Graphics& gfx);
	void Update(float dt) noexcept;

	Mesh* mesh;
	AnimationController* controller;
	bool is_bind_pose;
};


