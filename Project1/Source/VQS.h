#pragma once
#include "Quaternion.h"

class VQS {
public:
	VQS();
	VQS(const dx::XMFLOAT3& _v, const Quaternion& _q, float _s);
	VQS Add(const VQS& b) const;
	VQS ScalarProduct(const float& c) const;
	dx::XMVECTOR Transform(const dx::XMVECTOR& r) const;
	VQS Concatenate(const VQS& b) const;
	
	void SetV(const dx::XMFLOAT3& _v);
	void SetQ(const dx::XMFLOAT3& _v);
	void SetQ(const dx::XMFLOAT4& _v);
	dx::XMMATRIX toMatrix() const;
	VQS InterpolateTo(const VQS& vqs_n, float t);
private:
	dx::XMFLOAT3 v;
	Quaternion q;
	float s;
};

