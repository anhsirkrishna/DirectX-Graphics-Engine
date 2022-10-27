#pragma once
class Texture : public Bindable {
public:
	Texture(Graphics& gfx, const char* file_path);
	void Bind(Graphics& gfx) noexcept override;

};

