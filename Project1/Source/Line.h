#pragma once

#include "TestObject.h"
#include "ConstantBuffers.h"

class Line : public DrawableBase<Line>
{
public:
	Line(Graphics& gfx, DirectX::XMFLOAT4 color);
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void Update(float dt) noexcept override;
private:
	struct PSColorConst
	{
		DirectX::XMFLOAT4 color;
	} colorConst;
	using ColorCbuf = PixelConstantBuffer<PSColorConst>;
private:
	// model transform
	DirectX::XMFLOAT3X3 mt;
};