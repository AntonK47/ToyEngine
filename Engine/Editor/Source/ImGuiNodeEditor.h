#pragma once 

#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include "IconsFontAwesome6.h"
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace ImGui
{
	inline void ToggleButton(const char* label, bool* toggleValue, const ImVec2& size = ImVec2(0, 0))
	{
		const auto color1 = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		const auto color2 = ImGui::GetStyle().Colors[ImGuiCol_Button];

		ImGui::PushID(1);

		if (*toggleValue)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, color1);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color2);
			if (ImGui::Button(label, size))
			{

				*toggleValue = false;
			}
			ImGui::PopStyleColor(2);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, color2);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color1);
			if (ImGui::Button(label, size))
			{
				*toggleValue = true;
			}
			ImGui::PopStyleColor(2);
		}
		ImGui::PopID();
	}
}