#include "Project_Physics.h"
#include "App.h"
#include "imgui/imgui.h"

Project_Physics::Project_Physics(App* _p_parent_app) :
	Project(_p_parent_app, "Physics Simulation"),
	physics_system(_p_parent_app->GetWindow().Gfx()) {

	//Stick dimensions
	float width = 4.0f;
	float height = 2.0f;
	float depth = 2.0f;

	//back face bottom left vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(-width, -height, -depth), 1.0f)
	);
	//back face bottom right vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(width, -height, -depth), 1.0f)
	);
	//back face top left vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(-width, height, -depth), 1.0f)
	);
	//back face top right vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(width, height, -depth), 1.0f)
	);
	//front face bottom left vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(-width, -height, depth), 1.0f)
	);
	//front face bottom right vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(width, -height, depth), 1.0f)
	);
	//front face top left vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(-width, height, depth), 1.0f)
	);
	//front face top right vertex
	obj_vertices.push_back(
		std::make_pair(dx::XMFLOAT3(width, height, depth), 1.0f)
	);

	physics_system.SetStickWidth(0.0f);
}

void Project_Physics::Setup() {
	physics_system.AddPhysicsObject(obj_vertices);
	physics_system.AddPhysicsObject(obj_vertices);
}

void Project_Physics::Enter() {
	Camera& cam = p_parent_app->GetCamera();
	cam.SetR(100.f);
}

void Project_Physics::Update(float dt) {
	ProjectControls();
	physics_system.Update(dt);
}

void Project_Physics::Draw() {
	physics_system.Draw();
}

void Project_Physics::ProjectControls() {
	if (ImGui::Begin("PhyProject controls")) {
		//=========================================
		if (physics_system.ObjectCount() == max_objects) {
			ImGui::Text("Max Object Limit");
		}
		else {
			if (ImGui::Button("Add Stick")) {
				physics_system.AddPhysicsObject(obj_vertices);
			}
		}
	}
	ImGui::End();
}
