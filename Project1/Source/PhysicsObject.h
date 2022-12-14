#pragma once
#include "PhysicsSystem.h"
#include "DrawableBase.h"

struct PhyVertex {
	DirectX::XMVECTOR position;
	DirectX::XMFLOAT3 r_i;
	float mass;
};

class PhysicsObject
{
public:
	PhysicsObject();
	PhysicsObject(std::vector<DirectX::XMFLOAT3> positions, std::vector<float> masses);
	PhysicsObject(const std::vector<std::pair<DirectX::XMFLOAT3, float>>& vertices);
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

	void Add(const PhysicsState& _other);

	/*
	* Gets the current positions of the vertices
	* Returns: const ref to vertex_positions
	*/
	const std::vector<DirectX::XMFLOAT3>& GetVertexPositions();

private:
	friend class PhysicsSystem;
	std::vector<PhyVertex> vertices;
	//The positions after each update loop
	std::vector<DirectX::XMFLOAT3> vertex_positions;
	float mass;
	
	//Center of mass before moving it to origin
	DirectX::XMVECTOR center_of_mass;
	//Intertial Tensor
	DirectX::XMFLOAT3X3 Iobj;
	//Intertial Tensor Inverse
	DirectX::XMFLOAT3X3 Iobj_inv;

	//x(t) == c(t)
	DirectX::XMFLOAT3 position;
	//R(t)
	DirectX::XMFLOAT3X3 rotational_positon;

	//Linear components
	DirectX::XMFLOAT3 force;
	DirectX::XMFLOAT3 momentum;
	DirectX::XMFLOAT3 velocity;
	
	//Angular components
	DirectX::XMFLOAT3 torque;
	DirectX::XMFLOAT3 angular_momentum;
	DirectX::XMFLOAT3 angular_velocity;

	/*
	* Move the object (along with all vertices) so that the centre of mass 
	* corresponds with the origin.
	*/
	void MoveToOrigin();

	/*
	* Called by the physics manager to update all the position at the end of each frame
	* Returns : void
	*/
	void UpdatePositions();
};

