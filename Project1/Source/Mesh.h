#pragma once
#include "DrawableBase.h"
#include "FBXMesh.h"

class Mesh : public DrawableBase<Mesh>
{
public:
	Mesh(Graphics& gfx, FBXMesh& fbx_mesh);
	~Mesh();

	void Update(float dt) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept;
	// model transform
	DirectX::XMFLOAT3X3 mt;
};

