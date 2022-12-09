#pragma once
#include "ConstantBuffers.h"
#include "DrawableBase.h"

class Polyhedron : public DrawableBase<Polyhedron> {
public:
	Polyhedron(Graphics& gfx, 
		const std::vector<DirectX::XMFLOAT3>& vertices, DirectX::XMFLOAT4 color);
	DirectX::XMMATRIX GetTransformXM() const noexcept;
	void Update(float dt) noexcept override;
	void UpdateVertices(Graphics& gfx, const std::vector<DirectX::XMFLOAT3>& vertices);
	void SetColor(DirectX::XMFLOAT4 _color);
	void SyncMaterial(Graphics& gfx);
private:
	struct PSMaterialConstant
	{
		DirectX::XMFLOAT4 color;
	} materialConstants;
	using MaterialCbuf = PixelConstantBuffer<PSMaterialConstant>;
};

