#include "MaterialEditorColorNode.h"
#include <format>

using namespace toy::editor;
using namespace toy::editor::resolver;
using namespace toy::editor::resolver::glsl;

inline ColorNode::ColorNode()
{
	auto color = Pin{};
	color.id = ed::PinId{ core::UIDGenerator::generate() };
	color.name = "color";
	color.valueType = PinType::colorType;

	const auto nodeId = core::UIDGenerator::generate();


	id = ed::NodeId{ nodeId };
	title = "Color";
	outputPins.push_back(color);
	nodeColor = getTypePinColor(PinType::colorType);
}

inline void toy::editor::ColorNode::draw()
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::colorType)));

	const char* items[] = { "RGB", "HSV" };
	int selected = (int)selectedColorSpace;

	ImGui::PushID(id.Get());


	ImGui::SetNextItemWidth(width);
	if (ImGui::Button(items[selected], ImVec2(width, 0)))
	{
		openColorSpaceDropDown = true;
	}

	auto flags = ImGuiColorEditFlags_NoPicker
		| ImGuiColorEditFlags_NoOptions
		| ImGuiColorEditFlags_NoTooltip
		| ImGuiColorEditFlags_NoLabel
		| ImGuiColorEditFlags_NoDragDrop
		| ImGuiColorEditFlags_NoSmallPreview;

	auto colorRGB = ImVec4(color[0], color[1], color[2], color[3]);

	switch (selectedColorSpace)
	{
	case ColorSpace::RGBA:
		flags |= ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB;
		break;
	case ColorSpace::HSV:
		flags |= ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_DisplayHSV;
		break;
	default:
		break;
	}

	auto& style = ed::GetStyle();
	const auto colorButtonWidth = style.NodePadding.x + 20;
	ImGui::SetNextItemWidth(width - colorButtonWidth - ImGui::GetStyle().ItemSpacing.x);
	ImGui::ColorEdit4("##current", color.data(), flags);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(colorButtonWidth);
	if (ImGui::ColorButton("##current", colorRGB, flags))
	{
		openPickerPopup = true;
	}
	ImGui::PopID();

	ImGui::PopStyleColor();
}

inline void toy::editor::ColorNode::deferredDraw()
{
	ed::Suspend();

	const auto stringId = std::format("color_picker##{}", (int)id.Get());
	const auto colorSpaceId = std::format("color_space##{}", (int)id.Get());

	const char* items[] = { "RGB", "HSV" };
	int selected = (int)selectedColorSpace;

	if (openColorSpaceDropDown)
	{
		ImGui::OpenPopup(colorSpaceId.c_str());
		openColorSpaceDropDown = false;
	}


	if (ImGui::BeginPopup(colorSpaceId.c_str()))
	{
		if (ImGui::Button(items[0]))
		{
			if (selectedColorSpace != ColorSpace::RGBA)
			{
				float r, g, b;
				ImGui::ColorConvertHSVtoRGB(color[0], color[1], color[2], r, g, b);
				color[0] = r;
				color[1] = g;
				color[2] = b;
			}
			selectedColorSpace = ColorSpace::RGBA;

			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button(items[1])) {
			if (selectedColorSpace != ColorSpace::HSV)
			{
				float h, s, v;
				ImGui::ColorConvertRGBtoHSV(color[0], color[1], color[2], h, s, v);
				color[0] = h;
				color[1] = s;
				color[2] = v;
			}
			selectedColorSpace = ColorSpace::HSV;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}


	if (openPickerPopup)
	{
		ImGui::OpenPopup(stringId.c_str());
		openPickerPopup = false;
	}

	if (ImGui::BeginPopup(stringId.c_str()))
	{
		auto flags = ImGuiColorEditFlags_Uint8 |
			ImGuiColorEditFlags_DisplayRGB |
			ImGuiColorEditFlags_DisplayHSV |
			ImGuiColorEditFlags_DisplayHex |
			ImGuiColorEditFlags_PickerHueBar;
		switch (selectedColorSpace)
		{
		case ColorSpace::RGBA:
			flags |= ImGuiColorEditFlags_InputRGB;
			break;
		case ColorSpace::HSV:
			flags |= ImGuiColorEditFlags_InputHSV;
			break;
		default:
			break;
		}
		ImGui::ColorPicker4("", color.data(), flags);
		ImGui::EndPopup();
	}


	ed::Resume();
}

auto ColorNodeGlslResolver::resolve(core::u8 outputPinIndex) -> PinResolveResult*
{
	return new PinResolveResult;
}
