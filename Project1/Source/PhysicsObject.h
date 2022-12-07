#pragma once

#include "PhysicsSystem.h"

struct PhyVertex {
	DirectX::XMVECTOR position;
	DirectX::XMFLOAT3 r_i;
	float mass;
};

class PhysicsObject
{
private:
	friend class PhysicsSystem;
	std::vector<PhyVertex> vertices;
	float mass;
	DirectX::XMVECTOR center_of_mass;
	DirectX::XMMATRIX Iobj;
	DirectX::XMVECTOR force;
	DirectX::XMVECTOR momentum;
	DirectX::XMVECTOR torque;
	DirectX::XMVECTOR angular_momentum;
public:
	PhysicsObject();
	
	/*
	* Adds a vertex to the object.
	* Recalculates the center of mass and total mass
	* Returns : void
	*/
	void AddVertex(const PhyVertex& new_vertex);

	/*
	* Calculate the Iobj matrix for the given object
	*/
	void CalculateIObj();
};

