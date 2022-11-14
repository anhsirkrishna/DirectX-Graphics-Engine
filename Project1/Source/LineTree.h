#pragma once
class LineTree : public DrawableBase<LineTree> {
public:
	LineTree(Graphics& gfx, const Skeleton& skeleton);
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void Update(float dt) noexcept override;
	void SetBoneTransform(unsigned int index, const DirectX::XMMATRIX& transform);
	void SyncBones(Graphics& gfx);
	void SetPosition(DirectX::XMFLOAT3 _pos);
	void SetRotation(const DirectX::XMMATRIX& _rot);
	void SetModelTransform();
private:
	// model transform
	DirectX::XMMATRIX mt;
	DirectX::XMFLOAT3 position;
	DirectX::XMMATRIX rotation;

	struct VSBonesConstant
	{
		DirectX::XMMATRIX bones_transform[96];
	} bones_cbuf;
	using BonesCbuf = VertexConstantBuffer<VSBonesConstant>;
};
