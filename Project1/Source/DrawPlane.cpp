#include "DrawPlane.h"
#include "BindableBase.h"
#include "Plane.h"
#include "Texture.h"
#include "Sampler.h"
#include "imgui/imgui.h"

DrawPlane::DrawPlane(Graphics& gfx, const wchar_t* tex_file_name) :
	mt(), pos(), rotation(), scale(1.0f) {
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT2 tex;
		};
		auto model = Plane::MakeSkinned<Vertex>();
		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"TexturedVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"TexturedPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",   0, DXGI_FORMAT_R32G32B32_FLOAT,0,0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Texcoord",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else {
		SetIndexFromStatic();
	}

	AddBind(std::make_unique<Texture>(gfx, tex_file_name));
	AddBind(std::make_unique<Sampler>(gfx));

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));
}

void DrawPlane::Update(float dt) noexcept {
	namespace dx = DirectX;

	mt = dx::XMMatrixMultiply(
		dx::XMMatrixScaling(scale, scale, scale),
		dx::XMMatrixMultiply(rotation,
		dx::XMMatrixTranslation(pos.x, pos.y, pos.z)));
}

DirectX::XMMATRIX DrawPlane::GetTransformXM() const noexcept {
	return mt;
}

void DrawPlane::SetPosition(const DirectX::XMFLOAT3& _pos) noexcept {
	pos = _pos;
}

void DrawPlane::SetRotation(const DirectX::XMMATRIX& _rot) noexcept {
	rotation = _rot;
}

void DrawPlane::SetScale(float _scale) noexcept {
	scale = _scale;
}