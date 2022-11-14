#pragma once
#include "DrawableBase.h"
#include "FBXMesh.h"
#include "ConstantBuffers.h"

class Mesh : public DrawableBase<Mesh>
{
public:
	Mesh(Graphics& gfx, FBXMesh& fbx_mesh);
	~Mesh();

	void Update(float dt) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept;
	void Reset();

	void SetPosition(DirectX::XMFLOAT3 _pos);
	void SetRotation(const DirectX::XMMATRIX& _rot);

	// model transform
	DirectX::XMMATRIX mt;

	DirectX::XMFLOAT3 position;
	DirectX::XMMATRIX rotation;

	struct VSBonesConstant
	{
		DirectX::XMMATRIX bones_transform[96];
	} bones_cbuf;
	using BonesCbuf = VertexConstantBuffer<VSBonesConstant>;
	void SyncBones(Graphics& gfx);
};

