#pragma once
#include <DirectXMath.h>
namespace dx = DirectX;

class Quaternion {
public:
	Quaternion();
	Quaternion(float s, dx::XMFLOAT3 v);
	Quaternion(const dx::XMMATRIX& _mat);
	Quaternion(const dx::XMVECTOR& q);
	Quaternion Add(const Quaternion& b) const;
	Quaternion ScalarProduct(const float& c) const;
	float DotProduct(const Quaternion& b) const;
	Quaternion Concatenate(const Quaternion& b) const;
	Quaternion Conjugate() const;
	float Magnitude() const;
	void SetIdentity();
	Quaternion Inverse() const;

	dx::XMVECTOR toVector() const;
	dx::XMVECTOR Rotate(const dx::XMVECTOR& p) const;
	dx::XMMATRIX toMatrix() const;
	static Quaternion fromMatrix(const dx::XMMATRIX& _mat);

	Quaternion InterpolateTo(const Quaternion& q_n, float t);
private:
	float s;
	dx::XMFLOAT3 v;
};
