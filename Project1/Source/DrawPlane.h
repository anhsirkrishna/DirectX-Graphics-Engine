#pragma once
#include "DrawableBase.h"

class DrawPlane : public DrawableBase<DrawPlane> {
public:
	DrawPlane(Graphics& gfx, const wchar_t* tex_file_name);
	void Update(float dt) noexcept override;
	void SetPosition(const DirectX::XMFLOAT3& _pos) noexcept;
	void SetRotation(const DirectX::XMMATRIX& _rot) noexcept;
	void SetScale(float _scale) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
private:
	// model transform
	DirectX::XMMATRIX mt;
	DirectX::XMFLOAT3 pos;
	DirectX::XMMATRIX rotation;
	float scale;
};

