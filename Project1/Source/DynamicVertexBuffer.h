#pragma once
#include "Bindable.h"

class DynamicVertexBuffer : public Bindable {
public:
	template<class V>
	DynamicVertexBuffer(Graphics& gfx, const std::vector<V>& vertices) :
		stride(sizeof(V)) {
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(sizeof(V) * vertices.size());
		bd.StructureByteStride = sizeof(V);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices.data();
		GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer);
	}
	void Bind(Graphics& gfx) noexcept override;
	
	/*
	* Maps the dynamic vertex buffer.
	* Writes vertex data to it.
	* Unmaps the buffer
	*/
	template<class V>
	void Update(Graphics& gfx, const std::vector<V>& vertices);
	
protected:
	UINT stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
};

template<class V>
inline void DynamicVertexBuffer::Update(Graphics& gfx, const std::vector<V>& vertices) {
	D3D11_MAPPED_SUBRESOURCE msr;
	ZeroMemory(&msr, sizeof(D3D11_MAPPED_SUBRESOURCE));

	GetContext(gfx)->Map(
		pVertexBuffer.Get(), 0u,
		D3D11_MAP_WRITE_DISCARD, 0u,
		&msr
	);
	memcpy(msr.pData, vertices.data(), sizeof(vertices) * stride);
	GetContext(gfx)->Unmap(pVertexBuffer.Get(), 0u);
}
