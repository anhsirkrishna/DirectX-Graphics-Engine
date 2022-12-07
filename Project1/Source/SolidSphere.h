#pragma once
#include "DrawableBase.h"
class SolidSphere : public DrawableBase<SolidSphere> {
public:
	SolidSphere(Graphics& gfx, float radius);
	void Update(float dt) noexcept override;
	void SetPosition(DirectX::XMFLOAT3 _pos) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SetColor(Graphics& gfx, DirectX::XMFLOAT3 _color) noexcept;
private:
	struct PSColorConstant
	{
		DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
		float padding;
	} colorConst;
	DirectX::XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
};

