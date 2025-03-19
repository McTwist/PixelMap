#pragma once

#include <imgui.h>

#include <string>

namespace ImGui
{
	void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
	void EndGroupPanel();

	bool InputText(const char * label, std::string & str, ImGuiInputTextFlags flags = 0);
}
