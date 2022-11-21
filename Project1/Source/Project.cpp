#include "Project.h"

Project::Project(App* _p_parent_app, const std::string& project_name) : 
	p_parent_app(_p_parent_app), name(project_name) {
	
}

const std::string& Project::GetName() const {
	return name;
}
