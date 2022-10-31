#pragma once
class Curve : public DrawableBase<Curve> {
public:
	Curve(Graphics& gfx, const std::vector<dx::XMFLOAT3>& curve_points);
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void Update(float dt) noexcept override;
	void UpdateVertices(Graphics& gfx, const std::vector<dx::XMFLOAT3>& curve_points);
	void SetPosition(DirectX::XMFLOAT3 _pos);
	void SetModelTransform();
private:
	// model transform
	DirectX::XMFLOAT4X4 mt;
	DirectX::XMFLOAT3 position;
};

