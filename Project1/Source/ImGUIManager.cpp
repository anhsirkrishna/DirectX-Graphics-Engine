#include "ImGUIManager.h"
#include "imgui/imgui.h"

ImGUIManager::ImGUIManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

ImGUIManager::~ImGUIManager()
{
	ImGui::DestroyContext();
}
