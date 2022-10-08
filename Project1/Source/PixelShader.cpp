#include "PixelShader.h"

/*
* Reads in a compiled HLSL shader into a bytecode Blob
* Creates a Pixel shader using the Blob
*/
PixelShader::PixelShader(Graphics& gfx, const std::wstring& path) {
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
	D3DReadFileToBlob(path.c_str(), &pBytecodeBlob);
	GetDevice(gfx)->CreatePixelShader(
		pBytecodeBlob->GetBufferPointer(),
		pBytecodeBlob->GetBufferSize(),
		nullptr,
		&pPixelShader
	);
}

/*
* Sets this pixel shader as the active vertex shader
*/
void PixelShader::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->PSSetShader(pPixelShader.Get(), nullptr, 0);
}
