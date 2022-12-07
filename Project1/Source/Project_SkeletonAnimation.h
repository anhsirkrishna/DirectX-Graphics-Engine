#pragma once
#include "Project.h"
class Project_SkeletonAnimation : public Project {
public:
	Project_SkeletonAnimation(App* _p_parent_app);
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
private:
	std::vector<std::unique_ptr<Model>> draw_models;
	unsigned int active_model;
};

