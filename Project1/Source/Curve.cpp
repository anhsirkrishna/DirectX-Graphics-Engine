#include "DrawableBase.h"
#include "Path.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Topology.h"
#include "TransformCBuf.h"
#include "DynamicVertexBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Curve.h"

Curve::Curve(Graphics& gfx, const std::vector<dx::XMFLOAT3>& curve_points) {
	if (!IsStaticInitialized()) {
		auto pvs = std::make_unique<VertexShader>(gfx, L"CurveVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"CurvePS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "point_pos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
	}

	std::vector<int> vertex_indices;
	
	for (unsigned int indx = 0; indx < curve_points.size()-1; indx++) {
		vertex_indices.push_back(indx);
		vertex_indices.push_back(indx + 1);
	}

	AddBind(std::make_unique<DynamicVertexBuffer>(gfx, curve_points));
	AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, vertex_indices));

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));
	SetPosition(dx::XMFLOAT3(0.0f, -2.5f, 0.0f));
	// model deformation transform (per instance, not stored as bind)
	SetModelTransform();
}

DirectX::XMMATRIX Curve::GetTransformXM() const noexcept {
	return dx::XMLoadFloat4x4(&mt);
}

void Curve::Update(float dt) noexcept {
	SetModelTransform();
}

void Curve::UpdateVertices(Graphics& gfx, const std::vector<dx::XMFLOAT3>& curve_points) {
	auto pConstVB = QueryBindable<DynamicVertexBuffer>();
	assert(pConstVB != nullptr);
	pConstVB->Update(gfx, curve_points);
}

void Curve::SetPosition(DirectX::XMFLOAT3 _pos) {
	position = _pos;
}

void Curve::SetModelTransform() {
	dx::XMStoreFloat4x4(&mt,
		dx::XMMatrixTranslation(position.x, position.y, position.z));
}
