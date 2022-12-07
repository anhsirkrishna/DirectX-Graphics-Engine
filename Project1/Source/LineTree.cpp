#include "DrawableBase.h"
#include "VertexBuffer.h"
#include "ConstantBuffers.h"
#include "FBXAnimation.h"
#include "Animation.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Topology.h"
#include "TransformCBuf.h"
#include "LineTree.h"

#include <stack>

LineTree::LineTree(Graphics& gfx, const Skeleton& skeleton) {
	namespace dx = DirectX;

	if (!IsStaticInitialized()) {
		auto pvs = std::make_unique<VertexShader>(gfx, L"SkeletonVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"SkeletonPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Bone_index", 0, DXGI_FORMAT_R32G32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
	}

	struct LineTreeVertex
	{
		dx::XMUINT2 bone_index;
	};

	std::vector<LineTreeVertex> vertices;
	std::vector<int> vertex_indices;

	int bone_vertex_index = 0;
	LineTreeVertex vertex;
	for (auto parent_bone : skeleton.hierarchy) {
		for (auto bone : parent_bone->children) {
			//Bone consists of two vertices

			//First vertex is the parent bone
			vertex.bone_index.x = parent_bone->bone_indx;
			vertices.push_back(vertex);
			vertex_indices.push_back(bone_vertex_index++);

			//Second vertex is the child bone
			vertex.bone_index.x = bone->bone_indx;
			vertices.push_back(vertex);
			vertex_indices.push_back(bone_vertex_index++);
		}
	}
	AddBind(std::make_unique<VertexBuffer>(gfx, vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, vertex_indices));

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));

	AddBind(std::make_unique<BonesCbuf>(gfx, bones_cbuf, 1u));

	SetPosition(dx::XMFLOAT3(0.0f, -2.5f, 0.0f));
	// model deformation transform (per instance, not stored as bind)
	SetModelTransform();
}

DirectX::XMMATRIX LineTree::GetTransformXM() const noexcept {
	return mt;
}

void LineTree::Update(float dt) noexcept {
	SetModelTransform();
}

void LineTree::SetBoneTransform(unsigned int index, const DirectX::XMMATRIX& transform) {
	bones_cbuf.bones_transform[index] = transform;
}

void LineTree::SyncBones(Graphics& gfx) {
	auto pConstPS = QueryBindable<BonesCbuf>();
	assert(pConstPS != nullptr);
	pConstPS->Update(gfx, bones_cbuf);
}

void LineTree::SetPosition(DirectX::XMFLOAT3 _pos) {
	position = _pos;
}

void LineTree::SetRotation(const DirectX::XMMATRIX& _rot) {
	rotation = _rot;
}

void LineTree::SetModelTransform() {
	mt = dx::XMMatrixMultiply(rotation,
		dx::XMMatrixTranslation(position.x, position.y, position.z));
}
