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

	physics_system.SetStickWidth(width);
}

void Project_Physics::Setup() {
	physics_system.AddPhysicsObject(obj_vertices);
	physics_system.AddPhysicsObject(obj_vertices);
	physics_system.AddPhysicsObject(obj_vertices);
	physics_system.AddPhysicsObject(obj_vertices);
	physics_system.AddPhysicsObject(obj_vertices);
	physics_system.AddPhysicsObject(obj_vertices);
}

void Project_Physics::Enter() {
	Camera& cam = p_parent_app->GetCamera();
	cam.SetR(100.f);
}

void Project_Physics::Update(float dt) {
	ProjectControls(dt);
	physics_system.Update(dt);
}

void Project_Physics::Draw() {
	physics_system.Draw();
}

void Project_Physics::ProjectControls(float dt) {
	Window& window_ref = p_parent_app->GetWindow();
	unsigned int indx = physics_system.selected_anchor_point;
	if (window_ref.keyboard.isKeyPressed(VK_RIGHT)) {
		physics_system.anchor_points[indx].x = dx::XMMin(80.0f, physics_system.anchor_points[indx].x + (dt * 50));
	}
	if (window_ref.keyboard.isKeyPressed(VK_LEFT)) {
		physics_system.anchor_points[indx].x = dx::XMMax(-80.0f, physics_system.anchor_points[indx].x - (dt * 50));
	}

	if (window_ref.keyboard.isKeyPressed(VK_UP)) {
		physics_system.anchor_points[indx].y = dx::XMMin(50.0f, physics_system.anchor_points[indx].y + (dt * 50));
	}
	if (window_ref.keyboard.isKeyPressed(VK_DOWN)) {
		physics_system.anchor_points[indx].y = dx::XMMax(-50.0f, physics_system.anchor_points[indx].y - (dt * 50));
	}
}
