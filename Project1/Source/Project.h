#pragma once
#include "App.h"

class Project
{
public:
	Project(App* _p_parent_app, const std::string& project_name);
	virtual void Setup() = 0;
	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;
	const std::string& GetName() const;
protected:
	App* p_parent_app;
	std::string name;
};

