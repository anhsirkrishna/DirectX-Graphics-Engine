#include "VQS.h"

//Lerp between two Vec3
dx::XMFLOAT3 Lerp(const dx::XMFLOAT3& v_0, const  dx::XMFLOAT3& v_n, float t) {
	return dx::XMFLOAT3(
		(1 - t) * v_0.x + t * v_n.x,
		(1 - t) * v_0.y + t * v_n.y,
		(1 - t) * v_0.z + t * v_n.z
	);
}

//Lerp between two floats
float Lerp(float v_0, float v_n, float t) {
	return (1 - t) * v_0 + t * v_n;
}

VQS::VQS() : v(0.0f, 0.0f, 0.0f), q(), s(1.0f) {
}

VQS::VQS(const dx::XMFLOAT3& _v, const Quaternion& _q, float _s) 
	:  v(_v), q(_q), s(_s) {
}

VQS VQS::Add(const VQS& b) const {
	return VQS(
		dx::XMFLOAT3(v.x + b.v.x, v.y + b.v.y, v.z + b.v.z),
		q.Add(b.q),
		s + b.s
	);
}

VQS VQS::ScalarProduct(const float& c) const {
	return VQS(
		dx::XMFLOAT3(v.x * c, v.y * c, v.z * c),
		q.ScalarProduct(c),
		s * c
	);
}

dx::XMVECTOR VQS::Transform(const dx::XMVECTOR& r) const {
	//Scale with s
	dx::XMVECTOR transformed_r = dx::XMVectorScale(r, s);
	//Rotate with q
	transformed_r = q.Rotate(transformed_r);
	//Translate with v
	transformed_r = dx::XMVectorAdd(transformed_r, dx::XMLoadFloat3(&v));
	return transformed_r;
}

VQS VQS::Concatenate(const VQS& b) const {
	dx::XMFLOAT3 t_v;
	//Rotated transform
	dx::XMStoreFloat3(&t_v, Transform(dx::XMLoadFloat3(&b.v)));
	return VQS(
		t_v,
		q.Concatenate(b.q),
		s*b.s
	);
}

void VQS::SetV(const dx::XMFLOAT3& _v) {
	v = _v;
}

void VQS::SetQ(const dx::XMFLOAT3& _v) {
	q = Quaternion(0, _v);
}

void VQS::SetQ(const dx::XMFLOAT4& _v) {
	q = Quaternion(_v.w,
		dx::XMFLOAT3(_v.x, _v.y, _v.z));
}

const Quaternion& VQS::GetQ() const {
	return q;
}

const dx::XMFLOAT3& VQS::GetV() const {
	return v;
}

float VQS::GetS() const {
	return s;
}

dx::XMMATRIX VQS::toMatrix() const {
	return dx::XMMatrixAffineTransformation(
		dx::XMVectorSet(s, s, s, 1.0f),
		dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		q.toVector(),
		dx::XMVectorSet(v.x, v.y, v.z, 1.0f)
	);
}

VQS VQS::InterpolateTo(const VQS& vqs_n, float t) {
	return VQS(
		Lerp(v, vqs_n.v, t),
		q.InterpolateTo(vqs_n.q, t), 
		Lerp(s, vqs_n.s, t));
}

