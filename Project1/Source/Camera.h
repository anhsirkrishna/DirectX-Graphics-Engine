#pragma once
#include "Graphics.h"
#include "Window.h"
#include <armadillo>

class Camera
{
public:
	DirectX::XMMATRIX GetMatrix() const noexcept;
	void SpawnCameraControls() noexcept;
	void Reset() noexcept;
	void Update(float dt) noexcept;
	void SetControlWindow(Window* _control_window);
	void SetR(float _r);
private:
	float r = 400.0f;
	float theta = 0.0f;
	float phi = DirectX::XMConvertToRadians(35.0f);
	
	float cam_posx = 0.0f;
	float cam_posz = 0.0f;

	float look_posx = 0.0f;
	float look_posz = 0.0f;
	float speed = 500.0f;
	float angular_speed = 1;
	Window* control_window;
};

