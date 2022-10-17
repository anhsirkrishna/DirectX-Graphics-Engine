#include "Line.h"
#include "BindableBase.h"

Line::Line(Graphics& gfx, DirectX::XMFLOAT4 color) : mt() {
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		struct Vertex
		{
			dx::XMFLOAT3 pos;
		};
		std::vector<Vertex> vertices;
		dx::XMFLOAT3 pos(0.0, 0.0, 0.0);
		vertices.push_back(Vertex{ pos });
		pos = dx::XMFLOAT3(1.0, 1.0, 1.0);
		vertices.push_back(Vertex{ pos });

		std::vector<int> indices;
		indices.push_back(0);
		indices.push_back(1);

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"SolidVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"SolidPS.cso"));

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
	}
	else
	{
		SetIndexFromStatic();
	}

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));

	colorConst.color = color;
	AddBind(std::make_unique<ColorCbuf>(gfx, colorConst));

	// model deformation transform (per instance, not stored as bind)
	dx::XMStoreFloat3x3(
		&mt,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

DirectX::XMMATRIX Line::GetTransformXM() const noexcept
{
	namespace dx = DirectX;
	return dx::XMLoadFloat3x3(&mt) * dx::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

void Line::Update(float dt) noexcept {
}
