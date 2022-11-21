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
	* Update the objects and drawables in the project
	* Performs the logic required for the logic
	*/
	void Update(float dt) override;
	/*
	* Draws the objects and drawables
	*/
	void Draw() override;

	/*
	* Control the sphere position
	*/
	void ControlSpherePos();
private:
	std::unique_ptr<Model> draw_model;
	std::unique_ptr<Curve> draw_path;
	std::unique_ptr<SolidSphere> target_sphere;
	
	dx::XMFLOAT3 target_position;
	float distance_threshold = 10.0f;
};

