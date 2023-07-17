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
#include <concepts>

namespace ed = ax::NodeEditor;

namespace toy::editor
{
	enum class PinType
	{
		scalarType,
		rangedScalarType,
		vector2Type,
		vector3Type,
		vector4Type,
		colorType
	};


	using Vec4Type = std::array<float, 4>;
	using Vec3Type = std::array<float, 3>;
	using Vec2Type = std::array<float, 2>;
	using ScalarType = float;

	struct RangedScalarType
	{
		float value{};
		float min{ std::numeric_limits<float>::min() };
		float man{ std::numeric_limits<float>::max() };
	};

	using ValueType = std::variant<Vec4Type, Vec3Type, Vec2Type, ScalarType, RangedScalarType>;

	struct Pin
	{
		ed::PinId id{};
		std::string name{};
		std::string description{};
		ImColor accentColor{};
		PinType valueType{};
		ValueType defaultValue{};
		bool isRequired{ true };
	};

	ImColor getTypePinColor(const PinType& type)
	{
		auto color = ImColor{};
		switch (type)
		{
		case PinType::scalarType:
			color = ImColor(150, 150, 150);
			break;
		case PinType::colorType:
			color = ImColor(140, 60, 50);
		default:
			break;
		}
		return color;
	}


	struct MaterialNode
	{
		ed::NodeId id{};
		std::string title{};
		std::string description{};
		ImColor nodeColor{};
		std::vector<Pin> inputPins{};
		std::vector<Pin> outputPins{};
		constexpr static float defaultWidth{ 200 };
		float width{ defaultWidth };

		virtual void draw(){};
		virtual void deferredDraw(){}
		virtual ~MaterialNode(){}
	};

	struct ScalarNode final : public MaterialNode
	{
		ScalarNode()
		{
			auto scalarPin = Pin{};
			scalarPin.id = ed::PinId{ core::UIDGenerator::generate() };
			scalarPin.name = "scalar";
			scalarPin.valueType = PinType::scalarType;

			const auto nodeId = core::UIDGenerator::generate();

			id = ed::NodeId{ nodeId };
			title = "Scalar";
			outputPins.push_back(scalarPin);
			nodeColor = getTypePinColor(PinType::scalarType);
		}

		void draw() override
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::scalarType)));
			ImGui::SetNextItemWidth(width);
			ImGui::PushID(id.Get());
			ImGui::DragFloat("" , &scalar, 0.1f, 0.0f,0.0f, "%0.1f");
			ImGui::PopID();
			ImGui::PopStyleColor();
		}

	private:
		float scalar{ 1 };
	};

	struct Vector4dNode final : public MaterialNode
	{

	};

	struct Vector3dNode final : public MaterialNode
	{

	};

	struct Vector3dDecompose final : public MaterialNode
	{

	};

	struct UnaryArithmeticNode final : public MaterialNode
	{
		enum class UniraArithmeticOperations
		{
			sign,
			reciprocal
		};
	};

	struct ArithmeticNode final : public MaterialNode
	{
		enum class ArithmeticOperations
		{
			addition,
			substraction,
			multiplication,
			division,
			dotproduct,
			crossproduct,
			vectorScalarMultiplication
		};

	private:
		float a;
		float b;
	};

	struct Texture2dNode final : public MaterialNode
	{

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
			color.valueType = PinType::colorType;

			const auto nodeId = core::UIDGenerator::generate();

			
			id = ed::NodeId{ nodeId };
			title = "Color";
			outputPins.push_back(color);
			nodeColor = getTypePinColor(PinType::colorType);
		}

		ColorSpace selectedColorSpace{ ColorSpace::RGBA };
		Vec4Type color{ 0.0f, 0.0f, 0.0f, 1.0f };
		bool openPickerPopup{ false };
		bool openColorSpaceDropDown{ false };

		void draw() override
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::colorType)));

			const char* items[] = { "RGB", "HSV" };
			int selected = (int)selectedColorSpace;

			ImGui::PushID(id.Get());
			
			
			ImGui::SetNextItemWidth(width);
			if (ImGui::Button(items[selected], ImVec2(width,0))) 
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
			case toy::editor::ColorNode::ColorSpace::RGBA:
				flags |= ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB;
				break;
			case toy::editor::ColorNode::ColorSpace::HSV:
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
			if(ImGui::ColorButton("##current", colorRGB, flags))
			{
				openPickerPopup = true;
			}
			ImGui::PopID();
			
			ImGui::PopStyleColor();
		}

		void deferredDraw() override
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
		auto deinitialize() -> void;
		template<std::derived_from<MaterialNode> T>
		auto registerMaterialNode(const std::vector<std::string> category)
		{

		}
	private:

		auto drawNode(MaterialNode& node) -> void;
		auto drawMaterialEditor() -> void;
		

		void onDrawGui() override;


		ed::NodeId selectedNodeId{};
		ed::PinId selectedPinId{};
		ed::LinkId selectedLinkId{};

		ed::PinId pulledInputPin_{};
		ed::PinId pulledOutputPin_{};

		ImVec2 openPopupPosition_{};

		ed::EditorContext* context_;
		bool isFirstFrame_ = true;
		std::vector<std::unique_ptr<MaterialNode>> nodes_;
		std::vector<Link> links_;

		std::unordered_map<core::u32, std::unique_ptr<MaterialNode>> graphModel_;


	};

}