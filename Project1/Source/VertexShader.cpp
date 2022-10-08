#include "VertexShader.h"

/*
* Reads in a compiled HLSL shader into a bytecode Blob
* Creates a vertex shader using the Blob
*/
VertexShader::VertexShader(Graphics& gfx, const std::wstring& path) {
	D3DReadFileToBlob(path.c_str(), &pBytecodeBlob);
	GetDevice(gfx)->CreateVertexShader(
		pBytecodeBlob->GetBufferPointer(),
		pBytecodeBlob->GetBufferSize(),
		nullptr,
		&pVertexShader
	);
}

/*
* Sets this vertex shader as the active vertex shader
*/
void VertexShader::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
}

ID3DBlob* VertexShader::GetBytecode() const noexcept
{
	return pBytecodeBlob.Get();
}
