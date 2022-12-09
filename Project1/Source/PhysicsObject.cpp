#include "Graphics.h"
#include "PhysicsObject.h"

namespace dx = DirectX;

void PhysicsObject::MoveToOrigin() {
	for (auto& vertex : vertices) {
		vertex.position = dx::XMVectorSubtract(
			vertex.position, 
			dx::XMVectorNegate(center_of_mass));

		//Calculate ri with the new COM
		dx::XMStoreFloat3(&vertex.r_i, vertex.position);
	}
}

void PhysicsObject::UpdatePositions() {
	unsigned int indx = 0;
	for (auto& vertex : vertices) {
		dx::XMVECTOR updated_pos =
			dx::XMVectorAdd(
				dx::XMVector3Transform(vertex.position, rotational_positon),
				position);

		dx::XMStoreFloat3(&vertex_positions[indx], updated_pos);
		indx++;
	}
}

PhysicsObject::PhysicsObject() :
	vertices(), mass(0), center_of_mass(), 
	Iobj(dx::XMMatrixIdentity()), Iobj_inv(dx::XMMatrixIdentity()),
	position(dx::XMVectorZero()), rotational_positon(dx::XMMatrixIdentity()),
	force(dx::XMVectorZero()), momentum(dx::XMVectorZero()), velocity(dx::XMVectorZero()),
	torque(dx::XMVectorZero()), angular_momentum(dx::XMVectorZero()), angular_velocity(dx::XMVectorZero()) {
}

PhysicsObject::PhysicsObject(std::vector<DirectX::XMFLOAT3> positions, std::vector<float> masses) :
	PhysicsObject() {
	unsigned int indx = 0;
	for (auto& pos : positions) {
		AddVertex(PhyVertex{ .position = dx::XMLoadFloat3(&pos), .mass = masses[indx] });
		indx++;
	}
	MoveToOrigin();

	vertex_positions.resize(vertices.size());
}

PhysicsObject::PhysicsObject(const std::vector<std::pair<DirectX::XMFLOAT3, float>>& vertices) :
	PhysicsObject()  {
	for (auto& vertex : vertices) {
		AddVertex(PhyVertex{ .position = dx::XMLoadFloat3(&vertex.first), .mass = vertex.second });
	}
	MoveToOrigin();

	vertex_positions.resize(vertices.size());
}

/*
* Calculate the Iobj matrix for the given object
*/
void PhysicsObject::CalculateIObj() {
	float Ixx{0.0f}, Iyy{ 0.0f }, Izz{ 0.0f }, Ixy{ 0.0f }, Ixz{ 0.0f }, Iyz{ 0.0f };
	for (auto& vertex : vertices) {
		Ixx += vertex.mass * ((vertex.r_i.y * vertex.r_i.y) + (vertex.r_i.z * vertex.r_i.z));
		Iyy += vertex.mass * ((vertex.r_i.x * vertex.r_i.x) + (vertex.r_i.z * vertex.r_i.z));
		Izz += vertex.mass * ((vertex.r_i.x * vertex.r_i.x) + (vertex.r_i.y * vertex.r_i.y));

		Ixy += vertex.mass * (vertex.r_i.x * vertex.r_i.y);
		Ixz += vertex.mass * (vertex.r_i.x * vertex.r_i.z);
		Iyz += vertex.mass * (vertex.r_i.y * vertex.r_i.y);
	}

	Iobj = dx::XMMATRIX(
		 Ixx, -Ixy, -Ixz, 0,
		-Ixy,  Iyy, -Iyz, 0,
		-Ixz, -Iyz, -Izz, 0,
		0, 0, 0, 0
	);

	Iobj_inv = dx::XMMatrixInverse(nullptr, Iobj);
}

void PhysicsObject::Add(const PhysicsState& _other) {
	position = dx::XMVectorAdd(position, _other.c);
	rotational_positon = rotational_positon + _other.R;
	momentum = dx::XMVectorAdd(momentum, _other.P);
	angular_momentum = dx::XMVectorAdd(angular_momentum, _other.L);
}

dx::XMVECTOR PhysicsObject::GetQi(unsigned int i) {
	return dx::XMVectorAdd(
		dx::XMVector3Transform(vertices[i].position, rotational_positon),
		position);
}

const std::vector<DirectX::XMFLOAT3>& PhysicsObject::GetVertexPositions() {
	return vertex_positions;
}


void PhysicsObject::AddVertex(const PhyVertex& new_vertex) {
	vertices.push_back(new_vertex);

	//Recalculate the center of mass
	center_of_mass = dx::XMVectorScale(center_of_mass, mass);
	mass += new_vertex.mass;
	
	center_of_mass =
		dx::XMVectorAdd(
			center_of_mass,
			(dx::XMVectorScale(new_vertex.position, new_vertex.mass)));
	center_of_mass = dx::XMVectorScale(center_of_mass, 1.0f / mass);
}
