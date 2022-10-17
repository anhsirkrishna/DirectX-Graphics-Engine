#include "Mesh.h"
#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "Topology.h"
#include "TransformCBuf.h"
#include <memory>

Mesh::Mesh(Graphics& gfx, FBXMesh& fbx_mesh) {
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		struct SkinnedModelVertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 norm;
			dx::XMFLOAT2 tex;
			dx::XMFLOAT4 w;
			dx::XMINT4 i;
		};

		FBXMeshVertices& mesh = fbx_mesh.mesh;
		SkinData& skin = fbx_mesh.skin;

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, mesh.processed_indices));

		std::vector<SkinnedModelVertex> vertices;
		for (unsigned int i = 0; i < mesh.processed_vertices.size(); ++i) {
			Vertex& v = mesh.processed_vertices[i];
			int originalPositionIndex = mesh.source_indices[i].posIndex;
			WeightVector& weights = skin.point_weights[originalPositionIndex];
			SkinnedModelVertex vertex;
			vertex.pos = dx::XMFLOAT3(v.point[0], v.point[1], v.point[2]);
			vertex.norm = dx::XMFLOAT3(v.normal[0], v.normal[1], v.normal[2]);
			vertex.tex = dx::XMFLOAT2(v.texture[0], 1.0f - v.texture[1]);
			vertex.w = dx::XMFLOAT4(weights[0].weight, weights[1].weight, weights[2].weight, weights[3].weight);
			vertex.i = dx::XMINT4(weights[0].index, weights[1].index, weights[2].index, weights[3].index);
			vertices.push_back(vertex);
		}

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"AnimatedVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"AnimatedPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Normal",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Texcoord",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Weights",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Weightindex",0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		SetIndexFromStatic();
	}

	AddBind(std::make_unique<TransformCBuf>(gfx, *this));

	// model deformation transform (per instance, not stored as bind)
	dx::XMStoreFloat3x3( &mt, dx::XMMatrixIdentity());
}


Mesh::~Mesh()
{
}

void Mesh::Update(float dt) noexcept {
}

DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept {
	namespace dx = DirectX;
	return dx::XMLoadFloat3x3(&mt);
}