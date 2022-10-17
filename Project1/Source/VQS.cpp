#include "VQS.h"

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

