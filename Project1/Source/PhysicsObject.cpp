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
				dx::XMVector3Transform(vertex.position, dx::XMLoadFloat3x3(&rotational_positon)),
				dx::XMLoadFloat3(&position));

		dx::XMStoreFloat3(&vertex_positions[indx], updated_pos);
		indx++;
	}
}

PhysicsObject::PhysicsObject() :
	vertices(), mass(0), center_of_mass(), 
	Iobj(), Iobj_inv(),
	position(), rotational_positon(),
	force(), momentum(), velocity(),
	torque(), angular_momentum(), angular_velocity() {
	dx::XMStoreFloat3x3(&rotational_positon, dx::XMMatrixIdentity());
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
	CalculateIObj();
}

PhysicsObject::PhysicsObject(const std::vector<std::pair<DirectX::XMFLOAT3, float>>& vertices) :
	PhysicsObject()  {
	for (auto& vertex : vertices) {
		AddVertex(PhyVertex{ .position = dx::XMLoadFloat3(&vertex.first), .mass = vertex.second });
	}
	MoveToOrigin();

	vertex_positions.resize(vertices.size());
	CalculateIObj();
}

/*
* Calculate the Iobj matrix for the given object
*/
void PhysicsObject::CalculateIObj() {
	float Ixx{ 0.0f }, Iyy{ 0.0f }, Izz{ 0.0f }, Ixy{ 0.0f }, Ixz{ 0.0f }, Iyz{ 0.0f };
	for (auto& vertex : vertices) {
		Ixx += vertex.mass * ((vertex.r_i.y * vertex.r_i.y) + (vertex.r_i.z * vertex.r_i.z));
		Iyy += vertex.mass * ((vertex.r_i.x * vertex.r_i.x) + (vertex.r_i.z * vertex.r_i.z));
		Izz += vertex.mass * ((vertex.r_i.x * vertex.r_i.x) + (vertex.r_i.y * vertex.r_i.y));

		Ixy += vertex.mass * (vertex.r_i.x * vertex.r_i.y);
		Ixz += vertex.mass * (vertex.r_i.x * vertex.r_i.z);
		Iyz += vertex.mass * (vertex.r_i.y * vertex.r_i.z);
	}

	Iobj = dx::XMFLOAT3X3(
		 Ixx, -Ixy, -Ixz,
		-Ixy,  Iyy, -Iyz,
		-Ixz, -Iyz,  Izz
	);


	dx::XMStoreFloat3x3(
		&Iobj_inv,
		dx::XMMatrixInverse(nullptr, dx::XMLoadFloat3x3(&Iobj))
	);
		

	//==================================
	//The last vertex is the one with all positive dimensions
	/*float x = vertices.back().r_i.x;
	float y = vertices.back().r_i.y;
	float z = vertices.back().r_i.z;

	dx::XMMATRIX Iobj_x = mass * dx::XMMATRIX(
		y*y + z*z, 0, 0, 0,
		0, x*x + z*z, 0, 0,
		0, 0, x*x + y*y, 0,
		0, 0, 0, 1
	);

	dx::XMMATRIX Iobj_inv_x = (1.0f/mass) * dx::XMMATRIX(
		1.0f / (y * y + z * z), 0, 0, 0,
		0, 1.0f / (x * x + z * z), 0, 0,
		0, 0, 1.0f / (x * x + y * y), 0,
		0, 0, 0, 1
	);

	Iobj = Iobj_x;
	Iobj_inv = Iobj_inv_x;*/
	//==================================


}

void PhysicsObject::Add(const PhysicsState& _other) {
	dx::XMStoreFloat3(&position,
	 dx::XMVectorAdd(dx::XMLoadFloat3(&position), dx::XMLoadFloat3(&_other.c)));
	dx::XMStoreFloat3x3(&rotational_positon, 
		dx::XMLoadFloat3x3(&rotational_positon) + dx::XMLoadFloat3x3(&_other.R));
	dx::XMStoreFloat3(&momentum, 
		dx::XMVectorAdd(dx::XMLoadFloat3(&momentum), dx::XMLoadFloat3(&_other.P)));
	dx::XMStoreFloat3(&angular_momentum, 
		dx::XMVectorAdd(dx::XMLoadFloat3(&angular_momentum), dx::XMLoadFloat3(&_other.L)));
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
