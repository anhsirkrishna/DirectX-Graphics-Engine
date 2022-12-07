#include "Graphics.h"
#include "PhysicsObject.h"

namespace dx = DirectX;

PhysicsObject::PhysicsObject() : 
	vertices(), mass(0), center_of_mass() {
}

/*
* Calculate the Iobj matrix for the given object
*/
void PhysicsObject::CalculateIObj() {
	float Ixx, Iyy, Izz, Ixy, Ixz, Iyz;
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


	//Calculate ri with the new COM
	vertices.back().r_i = 
		dx::XMVectorSubtract(new_vertex.position, center_of_mass);
}
