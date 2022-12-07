#include "Graphics.h"
#include "PhysicsObject.h"
#include "PhysicsSystem.h"

namespace dx = DirectX;

void PhysicsSystem::EulerIntegrate(const PhysicsState& _s, float t, PhysicsState& _o) {
}

PhysicsSystem::PhysicsSystem() :
	elapsed_time(0) {
}

void PhysicsSystem::Derivative(
	const PhysicsObject& _obj,
	const PhysicsState& _s, float t, 
	PhysicsState& _o)
{
	float mass = _obj.mass;
	dx::XMVECTOR v = dx::XMVectorScale(_s.P, 1.0f/mass);
}
