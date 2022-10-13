#pragma once
#include <DirectXMath.h>
namespace dx = DirectX;

class Quaternion {
public:
	Quaternion();
	Quaternion(float s, dx::XMFLOAT3 v);
	Quaternion(const dx::XMMATRIX& _mat);
	Quaternion Add(const Quaternion& b) const;
	Quaternion ScalarProduct(const float& c) const;
	float DotProduct(const Quaternion& b) const;
	Quaternion Concatenate(const Quaternion& b) const;
	Quaternion Conjugate();
	float Magnitude();
	void SetIdentity();
	Quaternion Inverse();

	dx::XMVECTOR Rotate(const dx::XMVECTOR& p);
	dx::XMMATRIX toMatrix();
	static Quaternion fromMatrix(const dx::XMMATRIX& _mat);
private:
	float s;
	dx::XMFLOAT3 v;
};
