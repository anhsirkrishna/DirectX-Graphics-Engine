#pragma once
#include "Project.h"
#include "SolidSphere.h"

class Project_IK : public Project {
public:
	Project_IK(App* _p_parent_app);
	/*
	* Setup the required drawables and objects for the project
	*/
	void Setup() override;
	/*
	* Function called when this project is set as the active one
	*/
	void Enter() override;

	/*
	* Update the objects and drawables in the project
	* Performs the logic required for the project
	*/
	void Update(float dt) override;
	/*
	* Draws the objects and drawables
	*/
	void Draw() override;

	/*
	* Control the Project prameters
	*/
	void ProjectControls();

	/*
	* Control the position of the target
	*/
	void SphereControls();

	//Resets the simulation to the starting point
	void Reset();
private:
	std::unique_ptr<Model> draw_model;
	std::unique_ptr<Curve> draw_path;
	std::unique_ptr<SolidSphere> target_sphere;
	std::unique_ptr<SolidSphere> draw_control_point;
	std::unique_ptr<DrawPlane> draw_floor;
	std::vector<dx::XMFLOAT3> all_control_points;

	dx::XMFLOAT3 target_position;
	dx::XMFLOAT3 starting_position;
	float distance_threshold = 10.0f;
};

