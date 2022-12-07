#pragma once

struct PhysicsState
{
	DirectX::XMVECTOR c;
	DirectX::XMMATRIX R;
	DirectX::XMVECTOR P;
	DirectX::XMVECTOR L;
};

class PhysicsSystem {
private:
	float elapsed_time;
	enum class IntegrateMethod {
		EULER
	};
	IntegrateMethod use_method;

	std::vector<PhysicsObject> physics_objects;

	/*
	* Euler's method for integration by timesetp dt
	*/
	void EulerIntegrate(const PhysicsState& _s, float dt, PhysicsState& _o);
public:
	PhysicsSystem();
	void Update(float dt);
	void Derivative(const PhysicsObject& _obj, const PhysicsState& _s, float t, PhysicsState& _o);
	void Integrate(const PhysicsState& _s, float t, PhysicsState& _o);
};

