#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "Curve.h"

class Polyhedron;

struct PhysicsState
{
	//Position
	DirectX::XMVECTOR c;
	//Angular position
	DirectX::XMMATRIX R;

	//Momentum
	DirectX::XMVECTOR P;
	//Angular momentum
	DirectX::XMVECTOR L;

	void Scale(float t) {
		c = DirectX::XMVectorScale(c, t);
		R = R * t;
		P = DirectX::XMVectorScale(P, t);
		L = DirectX::XMVectorScale(L, t);
	}

	PhysicsState operator+(const PhysicsState& _other) const {
		PhysicsState ret_state;
		ret_state.c = DirectX::XMVectorAdd(c, _other.c);
		ret_state.R = R + _other.R;
		ret_state.P = DirectX::XMVectorAdd(P, _other.P);
		ret_state.L = DirectX::XMVectorAdd(L, _other.L);

		return ret_state;
	}

	PhysicsState operator*(float m) const {
		PhysicsState ret_state;
		ret_state.c = DirectX::XMVectorScale(c, m);
		ret_state.R = R * m;
		ret_state.P = DirectX::XMVectorScale(P, m);
		ret_state.L = DirectX::XMVectorScale(L, m);

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

