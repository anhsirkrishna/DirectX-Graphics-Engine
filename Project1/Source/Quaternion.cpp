#include "Quaternion.h"

Quaternion::Quaternion() : s(1.0f), v(0.0f, 0.0f, 0.0f) { }

Quaternion::Quaternion(float _s, dx::XMFLOAT3 _v) : s(_s), v(_v) { }

Quaternion Quaternion::Add(const Quaternion& b) const {
	float t_s = s + b.s;
	dx::XMFLOAT3 t_v(v.x+b.v.x, v.y+b.v.y, v.z + b.v.z);
	return Quaternion(t_s, t_v);
}

Quaternion Quaternion::ScalarProduct(const float& c) const {
	dx::XMFLOAT3 t_v(v.x * c, v.y * c, v.z * c);
	return Quaternion(c*s, t_v);
}

float Quaternion::DotProduct(const Quaternion& b) const {
	return (s*b.s) + (v.x + b.v.x) + (v.y + b.v.y) + (v.z + b.v.z);
}

Quaternion Quaternion::Concatenate(const Quaternion& b) const {
	float s1 = s;
	float s2 = b.s;

	dx::XMVECTOR v1 = dx::XMLoadFloat3(&v);
	dx::XMVECTOR v2 = dx::XMLoadFloat3(&b.v);

	//Calculate the scalar part with s1*s2 - v1.v2
	float t_s = (s1 * s2) - dx::XMVectorGetX(dx::XMVector3Dot(v1, v2));
	
	//Calculate the vector part with s1.v2 + s2.v2 + v1xv2
	dx::XMVECTOR t_v = dx::XMVectorAdd(dx::XMVectorScale(v2, s1), dx::XMVectorScale(v1, s2));
	t_v = dx::XMVectorAdd(t_v,
		dx::XMVector3Cross(v1, v2));
	
	//Return quaternion
	Quaternion ret_q;
	//Setting the scalar part
	ret_q.s = t_s;
	//Setting the vector part
	dx::XMStoreFloat3(&ret_q.v, t_v);

	return ret_q;
}

Quaternion Quaternion::Conjugate() {
	//Negating the vector part
	dx::XMVECTOR t_v = dx::XMLoadFloat3(&v);
	t_v = dx::XMVectorNegate(t_v);

	//Return quaternion
	Quaternion ret_q;
	//Setting the scalar part
	ret_q.s = s;
	//Setting the vector part
	dx::XMStoreFloat3(&ret_q.v, t_v);

	return ret_q;
}

float Quaternion::Magnitude() {
	//Squaring the vector part
	dx::XMVECTOR t_v = dx::XMLoadFloat3(&v);
	t_v = dx::XMVector3Dot(t_v, t_v);

	//Squaring the scalar part
	float t_s = s * s;

	float square = t_s + dx::XMVectorGetX(t_v);
	return sqrt(square);
}

void Quaternion::SetIdentity() {
	s = 1.0f;
	v.x = v.y = v.z = 0.0f;
}

Quaternion Quaternion::Inverse() {
	Quaternion conj = Conjugate();
	float mag = Magnitude();

	float t_s = s / mag;

	dx::XMVECTOR t_v = dx::XMLoadFloat3(&conj.v);
	t_v = dx::XMVectorScale(t_v, 1 / mag);
	//Return quaternion
	Quaternion ret_q;
	//Setting the scalar part
	ret_q.s = s;
	//Setting the vector part
	dx::XMStoreFloat3(&ret_q.v, t_v);

	return ret_q;
}

dx::XMVECTOR Quaternion::Rotate(const dx::XMVECTOR& p) {
	Quaternion result;
	result.s = dx::XMVectorGetW(p);
	dx::XMStoreFloat3(&result.v, p);

	result = Concatenate(result);
	result = result.Concatenate(Inverse());

	return dx::XMLoadFloat3(&result.v);
}
