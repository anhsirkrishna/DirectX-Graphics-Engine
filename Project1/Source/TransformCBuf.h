#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"

class TransformCBuf : public Bindable
{
private:
	struct Transforms
	{
		DirectX::XMMATRIX modelViewProj;
		DirectX::XMMATRIX model;
	};
public:
	TransformCBuf(Graphics& gfx, const Drawable& _parent, UINT slot = 0u);
	void Bind(Graphics& gfx) noexcept override;
private:
	static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
	const Drawable& parent;
};

