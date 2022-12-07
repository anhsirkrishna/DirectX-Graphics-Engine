#include "SolidSphere.h"
#include "BindableBase.h"
#include "Sphere.h"

SolidSphere::SolidSphere(Graphics& gfx, float radius) {
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		auto pvs = std::make_unique<VertexShader>(gfx, L"SolidVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"SolidPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	struct Vertex
	{
		dx::XMFLOAT3 pos;
	};
	auto model = Sphere::Make<Vertex>();
	model.Transform(dx::XMMatrixScaling(radius, radius, radius));
	AddBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, model.indices));

	AddBind(std::make_unique<PixelConstantBuffer<PSColorConstant>>(gfx, colorConst));

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));
}

void SolidSphere::Update(float dt) noexcept {}

void SolidSphere::SetPosition(DirectX::XMFLOAT3 _pos) noexcept {
	pos = _pos;
}

DirectX::XMMATRIX SolidSphere::GetTransformXM() const noexcept {
	return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void SolidSphere::SetColor(Graphics& gfx, DirectX::XMFLOAT3 _color) noexcept {
	colorConst.color = _color;
	auto pc_buf = QueryBindable<PixelConstantBuffer<PSColorConstant>>();
	pc_buf->Update(gfx, colorConst);
}
