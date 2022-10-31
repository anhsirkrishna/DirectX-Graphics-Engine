#pragma once
class LineTree : public DrawableBase<LineTree> {
public:
	LineTree(Graphics& gfx, const Skeleton& skeleton);
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void Update(float dt) noexcept override;
	void SetBoneTransform(unsigned int index, const DirectX::XMMATRIX& transform);
	void SyncBones(Graphics& gfx);
	void SetPosition(DirectX::XMFLOAT3 _pos);
	void SetModelTransform();
private:
	// model transform
	DirectX::XMFLOAT4X4 mt;
	DirectX::XMFLOAT3 position;
	struct VSBonesConstant
	{
		DirectX::XMMATRIX bones_transform[96];
	} bones_cbuf;
	using BonesCbuf = VertexConstantBuffer<VSBonesConstant>;
};
