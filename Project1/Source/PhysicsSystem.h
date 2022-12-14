#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "Curve.h"

class Polyhedron;

struct PhysicsState
{
	//Position
	DirectX::XMFLOAT3 c;
	//Angular position
	DirectX::XMFLOAT3X3 R;

	//Momentum
	DirectX::XMFLOAT3 P;
	//Angular momentum
	DirectX::XMFLOAT3 L;

	void Scale(float t) {
		DirectX::XMStoreFloat3(&c,
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&c), t));
		DirectX::XMStoreFloat3x3(&R, 
			DirectX::XMLoadFloat3x3(&R) * t);
		DirectX::XMStoreFloat3(&P,
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&P), t));
		DirectX::XMStoreFloat3(&L, 
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&L), t));
	}

	PhysicsState operator+(const PhysicsState& _other) const {
		PhysicsState ret_state;
		DirectX::XMStoreFloat3(&ret_state.c,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&c), DirectX::XMLoadFloat3(&_other.c)));
		DirectX::XMStoreFloat3x3(&ret_state.R,
			DirectX::XMLoadFloat3x3(&R) + DirectX::XMLoadFloat3x3(&_other.R));
		DirectX::XMStoreFloat3(&ret_state.P, 
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&P), DirectX::XMLoadFloat3(&_other.P)));
		DirectX::XMStoreFloat3(&ret_state.L, 
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&L), DirectX::XMLoadFloat3(&_other.L)));

		return ret_state;
	}

	PhysicsState operator*(float m) const {
		PhysicsState ret_state;
		DirectX::XMStoreFloat3(&ret_state.c, 
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&c), m));
		DirectX::XMStoreFloat3x3(&ret_state.R,
			DirectX::XMLoadFloat3x3(&R) * m );
		DirectX::XMStoreFloat3(&ret_state.P, 
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&P), m));
		DirectX::XMStoreFloat3(&ret_state.L, 
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&L), m));

		return ret_state;
	}
};

class PhysicsObject;

class PhysicsSystem {
public:
	PhysicsSystem(Graphics& gfx);
	~PhysicsSystem();

	//Add an object to the system
	void AddPhysicsObject(const std::vector<std::pair<DirectX::XMFLOAT3, float>>& vertices);

	//Called once every frame
	void Update(float dt);

	//Called once every frame to render each object
	void Draw();

	//Gets the number of physics objects
	size_t ObjectCount();

	//Sets the uniform stick width
	void SetStickWidth(float width);

	//Anchor points that do not respond to the physics system
	DirectX::XMFLOAT3 anchor_points[2];
	unsigned int selected_anchor_point;

private:
	Graphics& gfx_ref;

	float elapsed_time;
	enum class IntegrateMethod {
		EULER,
		RK4
	};
	IntegrateMethod use_method;

	bool apply_torque;

	//Objects on which to perform the physics calculations
	std::vector<PhysicsObject*> physics_objects;
	//Objects that represent the physics objects that will be rendered
	std::vector<Polyhedron*> draw_objects;

	DirectX::XMVECTOR gravity;
	std::vector<DirectX::XMVECTOR> global_forces;
	std::vector<DirectX::XMVECTOR> global_torques;
	DirectX::XMVECTOR total_global_force;
	DirectX::XMVECTOR total_global_torque;
	
	//Objects to render the anchor points
	SolidSphere anchor_sphere_1;
	SolidSphere anchor_sphere_2;
	
	std::unique_ptr<Curve> draw_spring;
	std::vector<DirectX::XMFLOAT3> draw_spring_vertices;

	//Stick width for stick-sprint system
	//All objects are assumed to be uniformly wide sticks.
	float stick_width;

	//Coefficients for damper-spring system
	float spring_coeff;
	float damper_coeff;

	/*
	* Euler's method for integration by timesetp dt
	*/
	void EulerIntegrate(PhysicsObject* _obj, float dt);

	/*
	* Runge-Kutta 4th order method for integration by timesetp dt
	*/
	void RK4Integrate(PhysicsObject* _obj, float dt);
	
	/*
	* Calculates all the global forces acting on the system
	* and stores it in total_global_force
	*/
	void CalculateAllForces();

	/*
	* Gets all the global Torques acting on the system
	* and stores it in total_global_torque
	*/
	void CalculateAllTorques();
	void Derivative(PhysicsState& _input, PhysicsObject* _obj, PhysicsState& _output);
	void Integrate(PhysicsObject* _obj, float dt);

	/*
	* Update the forces for the object with the Stick Spring system
	* Assumes each object is a uniformly sized stick
	*/
	void UpdateStickForces(PhysicsObject* _obj);

	//Calculate the velocities for all objects using the momentums for the current frame
	void CalculateVelocities();

	/*
	* Window to display and controle the Physics system parameters
	*/
	void SystemControls();

	/*
	* Update the positions of the springs being being drawn
	*/
	void UpdateDrawSprings();

	/*
	* Sets the anchor point position
	*/
	void SetAnchorPoint(unsigned int indx, DirectX::XMFLOAT3 point_pos);
};

