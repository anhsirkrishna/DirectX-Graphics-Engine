#include "Polyhedron.h"
#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "DrawableBase.h"
#include "imgui/imgui.h"
#include <string>

namespace dx = DirectX;

/*
* ~ Operator to convert a vector to a matrix
* Returns : XMMATRIX
*/
dx::XMMATRIX TildeOperator(dx::XMVECTOR _vec) {
	dx::XMFLOAT3 vec;
	dx::XMStoreFloat3(&vec, _vec);
	return dx::XMMATRIX(
		0, -vec.z, vec.y, 0,
		vec.z, 0, -vec.x, 0,
		-vec.y, vec.x, 0, 0,
		0, 0, 0, 1
	);
}

void PhysicsSystem::EulerIntegrate(PhysicsObject* _obj, float dt) {
	PhysicsState curr_state{
		.c = _obj->position,
		.R = _obj->rotational_positon,
		.P = _obj->momentum,
		.L = _obj->angular_momentum,
	};
	PhysicsState k1;
	Derivative(curr_state, _obj, k1);
	k1.Scale(dt);
	_obj->Add(k1);

	elapsed_time += dt;
}

void PhysicsSystem::RK4Integrate(PhysicsObject* _obj, float dt) {
	PhysicsState curr_state{
		.c = _obj->position,
		.R = _obj->rotational_positon,
		.P = _obj->momentum,
		.L = _obj->angular_momentum,
	};
	PhysicsState k1, k2, k3, k4;

	//k1 = dt * (y`(yi))
	Derivative(curr_state, _obj, k1);
	k1.Scale(dt);
	
	//k2 = dt * (y`(yi + k1/2))
	PhysicsState temp = k1 * 0.5;
	temp = curr_state + temp;
	Derivative(temp, _obj, k2);
	k2.Scale(dt);

	//k3 = dt * (y`(yi + k2/2))
	temp = k2 * 0.5;
	temp = curr_state + temp;
	Derivative(temp, _obj, k3);
	k3.Scale(dt);

	//k4 = dt * (y`(yi + k3))
	temp = curr_state + k3;
	Derivative(temp, _obj, k4);
	k4.Scale(dt);

	//yi+1 = yi + (k1 + 2k2 + 2k3 + k4)*(1/6)
	temp = (k1 + k2 * 2 + k3 * 2 + k4) * (1.0f / 6);
	_obj->Add(temp);

	elapsed_time += dt;
}

void PhysicsSystem::CalculateAllForces() {
	total_global_force = dx::XMVectorZero();
	for (auto& force : global_forces) {
		total_global_force = dx::XMVectorAdd(total_global_force, force);
	}
}

void PhysicsSystem::CalculateAllTorques() {
	total_global_torque = dx::XMVectorZero();
	for (auto& torque : global_torques) {
		total_global_torque = dx::XMVectorAdd(total_global_torque, torque);
	}
}

void PhysicsSystem::CalculateVelocities() {
	for (auto& obj : physics_objects) {
		//c'(t) = v(t) = P(t)/M
		obj->velocity = dx::XMVectorScale(obj->momentum, 1.0f / obj->mass);

		//I-inv(t) = R(t) * Iobj-inv * R(t)-trans
		dx::XMMATRIX I_inv =
			obj->rotational_positon * obj->Iobj_inv * dx::XMMatrixTranspose(obj->rotational_positon);
		I_inv = 
			dx::XMMatrixMultiply(
				dx::XMMatrixMultiply(
					obj->rotational_positon,
					obj->Iobj_inv
				),
				dx::XMMatrixTranspose(obj->rotational_positon));

		//w(t) = I-inv(t) * L(t)
		obj->angular_velocity = dx::XMVector3Transform(obj->angular_momentum, I_inv);
	}
	
}

void PhysicsSystem::SystemControls() {
	unsigned int indx = 0;
	if (ImGui::Begin("Physics controls")) {

		if (ImGui::BeginMenu("Integration Method")) {
			if (ImGui::MenuItem("Euler", "", use_method == IntegrateMethod::EULER)) use_method = IntegrateMethod::EULER;
			if (ImGui::MenuItem("Runge-Kutta 4th", "", use_method == IntegrateMethod::RK4)) use_method = IntegrateMethod::RK4;
			ImGui::EndMenu();
		}

		//=========================================
		ImGui::SliderFloat("Spring Coefficient", &spring_coeff, 0.1f, 1000.0f);
		ImGui::SliderFloat("Damper Coefficient", &damper_coeff, 0.1f, 1000.0f);
		ImGui::SliderFloat("Stick widht", &stick_width, 0.0f, 10.0f);
		//=========================================

		if (ImGui::Button("Add Global Force")) {
			global_forces.push_back(dx::XMVectorZero());
		}
		ImGui::Text("Global Forces :");
		indx = 0;
		for (auto& force : global_forces) {
			float force_val[3] = {
				dx::XMVectorGetX(force),
				dx::XMVectorGetY(force),
				dx::XMVectorGetZ(force)
			};
			std::string str = "Global Force " + std::to_string(indx);
			ImGui::SliderFloat3(str.c_str(), force_val, -100, 100);
			force =
				dx::XMVectorSet(force_val[0], force_val[1], force_val[2], 0.0f);
			indx++;
		}

		if (ImGui::Button("Add Global Torque")) {
			global_torques.push_back(dx::XMVectorZero());
		}
		ImGui::Text("Global Torques :");
		indx = 0;
		for (auto& torque : global_torques) {
			float torque_val[3] = {
				dx::XMVectorGetX(torque),
				dx::XMVectorGetY(torque),
				dx::XMVectorGetZ(torque)
			};
			std::string str = "Global Torque " + std::to_string(indx);
			ImGui::SliderFloat3(str.c_str(), torque_val, -10, 10);
			torque =
				dx::XMVectorSet(torque_val[0], torque_val[1], torque_val[2], 0.0f);
			indx++;
		}

		//=========================================
		float anchor_1_pos[3] = {
			anchor_points[0].x,
			anchor_points[0].y,
			anchor_points[0].z
		};
		ImGui::SliderFloat3("Anchor Point 1 pos", anchor_1_pos, -50, 50);
		anchor_points[0].x = anchor_1_pos[0];
		anchor_points[0].y = anchor_1_pos[1];
		anchor_points[0].z = anchor_1_pos[2];

		//=========================================
		float anchor_2_pos[3] = {
			anchor_points[1].x,
			anchor_points[1].y,
			anchor_points[1].z
		};
		ImGui::SliderFloat3("Anchor Point 2 pos", anchor_2_pos, -50, 50);
		anchor_points[1].x = anchor_2_pos[0];
		anchor_points[1].y = anchor_2_pos[1];
		anchor_points[1].z = anchor_2_pos[2];

		//=========================================

		indx = 0;
		for (auto& obj : physics_objects) {
			float obj_pos[3] = {
				dx::XMVectorGetX(obj->position),
				dx::XMVectorGetY(obj->position),
				dx::XMVectorGetZ(obj->position)
			};
			std::string str = "Object " + std::to_string(indx) +  "position";
			ImGui::SliderFloat3(str.c_str(), obj_pos, -100, 100);
			obj->position = 
				dx::XMVectorSet(obj_pos[0], obj_pos[1], obj_pos[2], 0.0f);
			indx++;
		}
	}
	ImGui::End();
}

PhysicsSystem::PhysicsSystem(Graphics& gfx) : gfx_ref(gfx),
	elapsed_time(0), use_method(IntegrateMethod::RK4), 
	anchor_sphere_1(gfx, 5), anchor_sphere_2(gfx, 5), 
	spring_coeff(100.0f), damper_coeff(100.0f) {

	//Add gravitational force
	gravity = dx::XMVectorSetY(dx::XMVectorZero(), -9.8f);

	anchor_points[0] = dx::XMFLOAT3(-20, 5, 0);
	anchor_points[1] = dx::XMFLOAT3(20, 5, 0);
}

PhysicsSystem::~PhysicsSystem() {
	for (auto& draw_obj : draw_objects) {
		delete draw_obj;
	}

	for (auto& phy_obj : physics_objects) {
		delete phy_obj;
	}
}

void PhysicsSystem::Derivative(PhysicsState& _input, PhysicsObject* _obj, PhysicsState& _output) {
	//c`(t) = v(t) = P(t)/M
	_output.c = dx::XMVectorScale(_input.P, 1.0f / _obj->mass);

	//I-inv(t) = R(t) * Iobj-inv * R(t)-trans
	dx::XMMATRIX I_inv =
		_input.R * _obj->Iobj_inv * dx::XMMatrixTranspose(_input.R);

	I_inv =
		dx::XMMatrixMultiply(
			dx::XMMatrixMultiply(
				_input.R,
				_obj->Iobj_inv
			),
			dx::XMMatrixTranspose(_input.R));

	//w(t) = I-inv(t) * L(t)
	dx::XMVECTOR omega = dx::XMVector3Transform(_input.L, I_inv);

	//R`(t) = ~w(t) * R(t)
	_output.R = TildeOperator(omega) * _input.R;
	_output.R = dx::XMMatrixMultiply(TildeOperator(omega), _input.R);

	//P`(t) = F(t)
	dx::XMVECTOR F = dx::XMVectorAdd(
		total_global_force,
		dx::XMVectorScale(gravity,
			_obj->mass)
	);
	_output.P = dx::XMVectorAdd(_obj->force, F);

	//L`(t) = T(t)
	_output.L = dx::XMVectorAdd(_obj->torque, total_global_torque);
}

void PhysicsSystem::Integrate(PhysicsObject* _obj, float dt){
	if (use_method == IntegrateMethod::EULER)
		EulerIntegrate(_obj, dt);
	else if (use_method == IntegrateMethod::RK4)
		RK4Integrate(_obj, dt);
}

void PhysicsSystem::UpdateStickForces(PhysicsObject* _obj) {
	unsigned int indx = 0;
	//Get the index of the physics object
	while (_obj != physics_objects[indx])
		indx++;

	dx::XMVECTOR fa, fb;
	dx::XMVECTOR ta, tb;

	dx::XMVECTOR qi_prev, qi_a, qi_b, qi_next;
	dx::XMVECTOR vi_prev, vi_a, vi_b, vi_next;

	dx::XMFLOAT3 stick_a, stick_b;
	stick_a.x = -stick_width;
	stick_a.y = 0;
	stick_a.z = 0;

	stick_b.x = stick_width;
	stick_b.y = 0;
	stick_b.z = 0;

	if (indx == 0) {
		//First object i-1 is anchor point
		qi_prev = dx::XMLoadFloat3(&anchor_points[0]);
			
		//qi_a = R(t) * [-x, 0, 0] + c(t)
		qi_next = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMLoadFloat3(&stick_a),
				physics_objects[indx + 1]->rotational_positon
			),
			physics_objects[indx + 1]->position
		);
			
		vi_prev = dx::XMVectorZero();
		vi_next = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMVector3Transform(
					dx::XMLoadFloat3(&stick_a),
					physics_objects[indx + 1]->rotational_positon),
				TildeOperator(physics_objects[indx + 1]->angular_velocity)),
			physics_objects[indx + 1]->velocity);
	}
	else if (indx == physics_objects.size() - 1)
	{
		//Last object i+1 is anchor point

		//qi_b = R(t) * [x, 0, 0] + c(t)
		qi_prev = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMLoadFloat3(&stick_b),
				physics_objects[indx - 1]->rotational_positon
			),
			physics_objects[indx - 1]->position
		);
		qi_next = dx::XMLoadFloat3(&anchor_points[1]);

		vi_prev = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMVector3Transform(
					dx::XMLoadFloat3(&stick_b),
					physics_objects[indx - 1]->rotational_positon),
				TildeOperator(physics_objects[indx - 1]->angular_velocity)),
			physics_objects[indx - 1]->velocity);
		vi_next = dx::XMVectorZero();
	}
	else
	{
		qi_prev = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMLoadFloat3(&stick_b),
				physics_objects[indx - 1]->rotational_positon
			),
			physics_objects[indx - 1]->position
		);

		qi_next = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMLoadFloat3(&stick_a),
				physics_objects[indx + 1]->rotational_positon
			),
			physics_objects[indx + 1]->position
		);

		vi_prev = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMVector3Transform(
					dx::XMLoadFloat3(&stick_b),
					physics_objects[indx - 1]->rotational_positon),
				TildeOperator(physics_objects[indx - 1]->angular_velocity)),
			physics_objects[indx - 1]->velocity);

		vi_next = dx::XMVectorAdd(
			dx::XMVector3Transform(
				dx::XMVector3Transform(
					dx::XMLoadFloat3(&stick_a),
					physics_objects[indx + 1]->rotational_positon),
				TildeOperator(physics_objects[indx + 1]->angular_velocity)),
			physics_objects[indx + 1]->velocity);

	}

	qi_a = dx::XMVectorAdd(
		dx::XMVector3Transform(
			dx::XMLoadFloat3(&stick_a),
			physics_objects[indx]->rotational_positon
		),
		physics_objects[indx]->position
	);

	qi_b = dx::XMVectorAdd(
		dx::XMVector3Transform(
			dx::XMLoadFloat3(&stick_b),
			physics_objects[indx]->rotational_positon
		),
		physics_objects[indx]->position
	);

	vi_a = dx::XMVectorAdd(
		dx::XMVector3Transform(
			dx::XMVector3Transform(
				dx::XMLoadFloat3(&stick_a),
				physics_objects[indx]->rotational_positon),
			TildeOperator(physics_objects[indx]->angular_velocity)),
		physics_objects[indx]->velocity);

	vi_b = dx::XMVectorAdd(
		dx::XMVector3Transform(
			dx::XMVector3Transform(
				dx::XMLoadFloat3(&stick_b),
				physics_objects[indx]->rotational_positon),
			TildeOperator(physics_objects[indx]->angular_velocity)),
		physics_objects[indx]->velocity);


	//Calculate linear forces

	fa = dx::XMVectorAdd(
		dx::XMVectorScale(dx::XMVectorSubtract(qi_prev, qi_a), spring_coeff),
		dx::XMVectorScale(dx::XMVectorSubtract(vi_prev, vi_a), damper_coeff));

	fb = dx::XMVectorAdd(
		dx::XMVectorScale(dx::XMVectorSubtract(qi_next, qi_b), spring_coeff),
		dx::XMVectorScale(dx::XMVectorSubtract(vi_next, vi_b), damper_coeff));

	_obj->force = dx::XMVectorAdd(fa, fb);


	//Calculate angular forces
	ta = dx::XMVector3Cross(
		dx::XMVectorSubtract(qi_a, physics_objects[indx]->position),
		fa);

	tb = dx::XMVector3Cross(
		dx::XMVectorSubtract(qi_b, physics_objects[indx]->position),
		fb);
	
	_obj->torque = dx::XMVectorAdd(ta, tb);
		
}

void PhysicsSystem::AddPhysicsObject(const std::vector<std::pair<dx::XMFLOAT3, float>>& vertices) {
	//Move the anchon points further apart to make room for the new object
	anchor_points[0].x -= 5;
	anchor_points[1].x += 5;


	PhysicsObject* phy_obj = new PhysicsObject(vertices);
	physics_objects.push_back(phy_obj);

	std::vector<dx::XMFLOAT3> draw_vertices;
	for (auto& vertex : vertices)
		draw_vertices.push_back(vertex.first);
	Polyhedron* draw_obj = new Polyhedron(gfx_ref, draw_vertices,
		dx::XMFLOAT4(162.0f / 255, 0.0f / 255, 255.0f / 255, 255.0f / 255));
	draw_objects.push_back(draw_obj);
}

void PhysicsSystem::Update(float dt) {
	SystemControls();

	//Ensure all global forces are updated
	CalculateAllForces();
	CalculateAllTorques();
	CalculateVelocities();

	for (auto& obj : physics_objects) {
		//Update the forces on the object using the stick-spring system
		UpdateStickForces(obj);
		Integrate(obj, dt);

		//Update the object positions with the new positions for this frame
		obj->UpdatePositions();
	}
}

void PhysicsSystem::Draw() {
	unsigned int indx = 0;
	for (auto& obj : draw_objects) {
		obj->UpdateVertices(gfx_ref,
			physics_objects[indx]->GetVertexPositions());
		obj->Draw(gfx_ref);
		indx++;
	}

	anchor_sphere_1.SetPosition(anchor_points[0]);
	anchor_sphere_2.SetPosition(anchor_points[1]);

	anchor_sphere_1.Draw(gfx_ref);
	anchor_sphere_2.Draw(gfx_ref);
}

size_t PhysicsSystem::ObjectCount() {
	return physics_objects.size();
}

void PhysicsSystem::SetStickWidth(float width) {
	stick_width = width;
}
