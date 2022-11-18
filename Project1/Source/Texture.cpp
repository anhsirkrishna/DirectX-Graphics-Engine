#include "Texture.h"
#include "WICTextureLoader.h"

Texture::Texture(Graphics& gfx, const wchar_t* filename) {
	HRESULT err = CreateWICTextureFromFile(
		GetDevice(gfx), GetContext(gfx), filename, nullptr, &pTextureView);
}

void Texture::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->PSSetShaderResources(0u, 1u, pTextureView.GetAddressOf());
}
