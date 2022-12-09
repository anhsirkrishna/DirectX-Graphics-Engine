#pragma once
#include "Project.h"
#include "PhysicsSystem.h"

class Project_Physics : public Project {
public:
	Project_Physics(App* _p_parent_app);
	/*
	* Setup the physics objects and add them to the physics system
	*/
	void Setup() override;

	/*
	* Function called when this project is set as the active one
	*/
	void Enter() override;

	/*
	* Update physics system
	* Performs the logic required for the project
	*/
	void Update(float dt) override;

	/*
	* Calls the PhysicsSystem.Draw()
	* Draws other required objects
	*/
	void Draw() override;

	/*
	* Control the Project prameters
	*/
	void ProjectControls();
private:
	PhysicsSystem physics_system;
	//Template for the physics object
	std::vector<std::pair<DirectX::XMFLOAT3, float>> obj_vertices;
	unsigned int max_objects = 6;
	unsigned int min_objects = 2;
};

