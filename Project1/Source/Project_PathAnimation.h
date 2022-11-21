#pragma once
#include "Project.h"

class Project_PathAnimation : public Project
{	
public:
	Project_PathAnimation(App* _p_parent_app);
	void Setup() override;
	void Update(float dt) override;
	void Draw() override;
private:
	std::unique_ptr<Animation> animation;
	std::unique_ptr<Model> draw_model;
	std::unique_ptr<Curve> draw_path;
};

