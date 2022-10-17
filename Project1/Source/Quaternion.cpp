#include "Quaternion.h"

Quaternion::Quaternion() : s(1.0f), v(0.0f, 0.0f, 0.0f) { }

Quaternion::Quaternion(float _s, dx::XMFLOAT3 _v) : s(_s), v(_v) { }

Quaternion::Quaternion(const dx::XMMATRIX& _mat) {
	Quaternion t_q = fromMatrix(_mat);
	s = t_q.s;
	v = t_q.v;
}

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

Quaternion Quaternion::Conjugate() const {
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

float Quaternion::Magnitude() const {
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

Quaternion Quaternion::Inverse() const {
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

dx::XMVECTOR Quaternion::Rotate(const dx::XMVECTOR& p) const {
	Quaternion result;
	result.s = dx::XMVectorGetW(p);
	dx::XMStoreFloat3(&result.v, p);

	result = Concatenate(result);
	result = result.Concatenate(Inverse());

	return dx::XMLoadFloat3(&result.v);
}

dx::XMMATRIX Quaternion::toMatrix() const {
	return dx::XMMatrixSet(
		1 - 2 * ((v.y * v.y) + (v.z * v.z)), //M00
		2 * ((v.x * v.y) - (s * v.z)), //M01
		2 * ((v.x * v.z) + (s * v.y)), //M02
		0, //M03
		2 * ((v.x * v.y) + (s * v.z)), //M10
		1 - 2 * ((v.x * v.x) + (v.z * v.z)), //M11
		2 * ((v.y * v.z) + (s * v.x)), //M12,
		0, //M13
		2 * ((v.x * v.z) + (s * v.y)), //M20
		2 * ((v.y * v.z) + (s * v.x)), //M21
		1 - 2 * ((v.x * v.x) + (v.y * v.y)), //M22
		0, //M23
		0, //M30
		0, //M31
		0, //M32
		1  //M33
	);
}

Quaternion Quaternion::fromMatrix(const dx::XMMATRIX& _mat) {
	dx::XMFLOAT4X4 tmp;
	dx::XMStoreFloat4x4(&tmp, _mat);
	float t_s = 0.5 * sqrt(tmp._11 + tmp._22 + tmp._33 + 1);

	float x = (tmp._32 - tmp._32) / (4 * t_s);
	float y = (tmp._13 - tmp._31) / (4 * t_s);
	float z = (tmp._21 - tmp._12) / (4 * t_s);

	return Quaternion(t_s, dx::XMFLOAT3(x, y, z));
}

//Slerp between two quaternions
Quaternion Quaternion::InterpolateTo(const Quaternion& q_n, float t) {
	float d_p = DotProduct(q_n);
	bool flip = false;
	if (d_p < 0)
	{
		d_p = -d_p;
	}
	float alpha = dx::XMScalarACos(d_p);
	float sin_alpha = dx::XMScalarSin(alpha);
	float sin_t_alpha = dx::XMScalarSin(t * alpha);
	float sin_t_m_alpha = dx::XMScalarSin((1 - t) * alpha);
	
	float t_s = ((s * sin_t_m_alpha) / sin_alpha) + ((q_n.s * sin_t_m_alpha) / sin_alpha);
	dx::XMFLOAT3 t_v(
		((v.x * sin_t_m_alpha) / sin_alpha) + ((q_n.v.x * sin_t_m_alpha) / sin_alpha),
		((v.y * sin_t_m_alpha) / sin_alpha) + ((q_n.v.y * sin_t_m_alpha) / sin_alpha),
		((v.z * sin_t_m_alpha) / sin_alpha) + ((q_n.v.z * sin_t_m_alpha) / sin_alpha));
	return Quaternion();
}
