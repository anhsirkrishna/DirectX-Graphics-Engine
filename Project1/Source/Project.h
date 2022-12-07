#pragma once
#include <string>

class App;

class Project
{
public:
	Project(App* _p_parent_app, const std::string& project_name);
	virtual void Setup() = 0;
	virtual void Enter() = 0;
	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;
	const std::string& GetName() const;
protected:
	App* p_parent_app;
	std::string name;
};

