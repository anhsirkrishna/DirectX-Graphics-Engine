#include "Polyhedron.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Topology.h"
#include "DynamicVertexBuffer.h"
#include "TransformCBuf.h"

Polyhedron::Polyhedron(Graphics& gfx, 
	const std::vector<DirectX::XMFLOAT3>& vertices, DirectX::XMFLOAT4 color) 
	: materialConstants{ .color = color } {
	if (!IsStaticInitialized()) {
		auto pvs = std::make_unique<VertexShader>(gfx, L"SolidVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"SolidPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	std::vector<int> vertex_indices{
		0, 2, 1, 2, 3, 1,
		1, 3, 5, 3, 7, 5,
		2, 6, 3, 3, 6, 7,
		4, 5, 7, 4, 7, 6,
		0, 4, 2, 2, 4, 6,
		0, 1, 4, 1, 5, 4
	};
	
	AddBind(std::make_unique<DynamicVertexBuffer>(gfx, vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, vertex_indices));

	materialConstants.color = color;
	AddBind(std::make_unique<MaterialCbuf>(gfx, materialConstants));

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));
}

DirectX::XMMATRIX Polyhedron::GetTransformXM() const noexcept {
	return DirectX::XMMatrixIdentity();
}

void Polyhedron::Update(float dt) noexcept {
}

void Polyhedron::UpdateVertices(Graphics& gfx, const std::vector<DirectX::XMFLOAT3>& vertices) {
	auto pConstVB = QueryBindable<DynamicVertexBuffer>();
	assert(pConstVB != nullptr);
	pConstVB->Update(gfx, vertices);
}


void Polyhedron::SetColor(DirectX::XMFLOAT4 _color) {
	materialConstants.color = _color;
}

void Polyhedron::SyncMaterial(Graphics& gfx) {
	auto pConstPS = QueryBindable<MaterialCbuf>();
	assert(pConstPS != nullptr);
	pConstPS->Update(gfx, materialConstants);
}
