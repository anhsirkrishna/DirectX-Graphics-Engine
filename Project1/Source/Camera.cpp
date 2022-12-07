#include "Camera.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

DirectX::XMMATRIX Camera::GetMatrix() const noexcept {
	const auto pos = dx::XMVector3Transform(
		dx::XMVectorSet(cam_posx, 0.0f, -r, 0.0f),
		dx::XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f)
	);
	return dx::XMMatrixLookAtLH(
		pos, dx::XMVectorSet(look_posx, 0.0f, look_posz, 0.0f),
		dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void Camera::SpawnCameraControls() noexcept {
	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("R", &r, 1.0f, 2000.0f, "%.1f");
		ImGui::SliderAngle("Theta", &theta, -180.0f, 180.0f);
		ImGui::SliderAngle("Phi", &phi, -89.0f, 89.0f);
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
		ImGui::SliderFloat("Speed", &speed, 1.0f, 2000.0f, "%.1f");
		ImGui::SliderFloat("Angular Speed", &angular_speed, 0.5f, 2.0f, "%.1f");
	}
	ImGui::End();
}

void Camera::Reset() noexcept {
	r = 400.0f;
	theta = 0.0f;
	phi = DirectX::XMConvertToRadians(35.0f);
	cam_posx = 0.0f;
	cam_posz = 0.0f;
	look_posx = 0.0f;
	look_posz = 0.0f;
	speed = 500.0f;
}

void Camera::Update(float dt) noexcept {
	if (control_window->keyboard.isKeyPressed(int('Q')))
		phi -= angular_speed * dt;
	if (control_window->keyboard.isKeyPressed(int('E')))
		phi += angular_speed * dt;

	if (control_window->keyboard.isKeyPressed(int('W')))
		r -= speed * dt;
	if (control_window->keyboard.isKeyPressed(int('S')))
		r += speed * dt;

	if (control_window->keyboard.isKeyPressed(int('A')))
		theta -= angular_speed * dt;
	if (control_window->keyboard.isKeyPressed(int('D')))
		theta += angular_speed * dt;
}

void Camera::SetControlWindow(Window* _control_window) {
	control_window = _control_window;
}

void Camera::SetR(float _r) {
	r = _r;
}
