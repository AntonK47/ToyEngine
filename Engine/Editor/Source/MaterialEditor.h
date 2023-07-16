#pragma once
#include "Editor.h"
#include "Core.h"

#include <string>
#include <vector>

#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include "IconsFontAwesome6.h"

#include <imgui_node_editor.h>
#include <unordered_map>
#include <variant>
#include <array>
#include <memory>
#include <format>

namespace ed = ax::NodeEditor;

namespace toy::editor
{
	enum class PinType
	{
		floatType,
		vector2Type,
		vector3Type,
		vector4Type
	};

	struct Pin
	{
		ed::PinId id{};
		std::string name{};
		std::string description{};
		ImColor accentColor{};
		PinType valueType{};
	};

	using Vec4Type = std::array<float, 4>;
	using Vec3Type = std::array<float, 3>;
	using Vec2Type = std::array<float, 2>;
	using FloatType = float;

	using ValueType = std::variant<Vec4Type, Vec3Type, Vec2Type, FloatType>;


	struct MaterialNode
	{
		ed::NodeId id{};
		std::string title{};
		std::string description{};
		ImColor accentColor{};
		ImColor headerColor{};
		std::vector<Pin> inputPins{};
		std::vector<Pin> outputPins{};
		constexpr static float defaultWidth{ 200 };
		float width{ defaultWidth };

		virtual void draw() = 0;
		virtual void deferredDraw(){}
		virtual ~MaterialNode(){}
	};

	struct ColorNode final : public MaterialNode
	{
		enum class ColorSpace
		{
			RGBA,
			HSV
		};

		ColorNode()
		{
			auto color = Pin{};
			color.id = ed::PinId{ core::UIDGenerator::generate() };
			color.name = "color";
			color.accentColor = ImColor(0, 150, 0);//<< vec2 type color
			color.valueType = PinType::vector4Type;

			const auto nodeId = core::UIDGenerator::generate();

			
			id = ed::NodeId{ nodeId };
			title = "Color";
			accentColor = ImColor(100, 0, 0);
			outputPins.push_back(color);
			headerColor = ImColor(100, 20, 10);
		}

		ColorSpace selectedColorSpace{ ColorSpace::RGBA };
		Vec4Type color{};
		bool openPickerPopup{ false };

		void draw() override
		{
			const char* items[] = { "RGB", "HSV" };
			int selected = (int)selectedColorSpace;

			ImGui::PushID(id.Get());
			ImGui::RadioButton(items[0], &selected, 0);
			ImGui::SameLine();
			ImGui::RadioButton(items[1], &selected, 1);


			auto flags = ImGuiColorEditFlags_NoPicker 
				| ImGuiColorEditFlags_NoOptions
				| ImGuiColorEditFlags_NoTooltip 
				| ImGuiColorEditFlags_NoLabel
				| ImGuiColorEditFlags_NoDragDrop
				| ImGuiColorEditFlags_NoSmallPreview;

			auto colorRGB = ImVec4(color[0], color[1], color[2], color[3]);

			switch (selectedColorSpace)
			{
			case toy::editor::ColorNode::ColorSpace::RGBA:
				flags |= ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB;
				break;
			case toy::editor::ColorNode::ColorSpace::HSV:
				flags |= ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_DisplayHSV;
				/*float r, g, b;
				ImGui::ColorConvertHSVtoRGB(color[0], color[1], color[2], r, g, b);
				colorRGB.x = r;
				colorRGB.y = g;
				colorRGB.z = b;*/
				break;
			default:
				break;
			}

			auto& style = ed::GetStyle();
			const auto colorButtonWidth = style.NodePadding.x + 20;
			ImGui::SetNextItemWidth(width - colorButtonWidth);
			ImGui::ColorEdit4("##current", color.data(), flags);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(colorButtonWidth);
			if(ImGui::ColorButton("##current", colorRGB, flags))
			{
				openPickerPopup = true;
			}
			ImGui::PopID();
			selectedColorSpace = (ColorSpace)selected;
		}

		void deferredDraw() override
		{
			ed::Suspend();

			const auto stringId = std::format("color_picker##{}", (int)id.Get());

			if (openPickerPopup) {
				ImGui::OpenPopup(stringId.c_str());
				openPickerPopup = false;
			}

			if (ImGui::BeginPopup(stringId.c_str()))
			{
				ImGui::ColorPicker4("", color.data());
				ImGui::EndPopup();
			}
				
			ed::Resume();
		}
	};

	struct Link
	{
		ed::LinkId id{};
		ed::PinId fromPinId{};
		ed::PinId toPinId{};
	};

	class MaterialEditor final : public Editor, Document
	{
	public:
		auto initialize() -> void;
	private:

		auto drawNode(MaterialNode& node) -> void;
		auto drawMaterialEditor() -> void;
		

		void onDrawGui() override;


		ed::NodeId selectedNodeId{};
		ed::PinId selectedPinId{};
		ed::LinkId selectedLinkId{};

		ed::PinId pulledInputPin_{};
		ed::PinId pulledOutputPin_{};

		ed::EditorContext* context_;
		bool isFirstFrame_ = true;
		std::vector<std::unique_ptr<MaterialNode>> nodes_;
		std::vector<Link> links_;

		std::unordered_map<core::u32, std::unique_ptr<MaterialNode>> graphModel_;
	};

}