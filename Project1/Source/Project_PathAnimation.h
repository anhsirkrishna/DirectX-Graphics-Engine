#pragma once
#include "Project.h"

class Project_PathAnimation : public Project
{	
public:
	Project_PathAnimation(App* _p_parent_app);
	void Enter() override;
	void Setup() override;
	void Update(float dt) override;
	void Draw() override;
private:
	std::unique_ptr<Animation> animation;
	std::unique_ptr<Model> draw_model;
	std::unique_ptr<Curve> draw_path;
	std::unique_ptr<SolidSphere> draw_control_point;
	std::unique_ptr<DrawPlane> draw_floor;
	std::vector<dx::XMFLOAT3> all_control_points;
};

