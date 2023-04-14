#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <string>
#include <vector>
#include <variant>
#include <format>
#include <optional>
#include <unordered_map>
#include <array>

#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include "IconsFontAwesome6.h"

#include <imgui_node_editor.h>
#include <Core.h>
#include <ValidationCommon.h>


namespace ed = ax::NodeEditor;

namespace toy::editor::materials
{

	enum class NodeType
	{
		none,
		color,
		materialOutput
	};

	using Vec4Type = std::array<float, 4>;
	using Vec3Type = std::array<float, 3>;
	using Vec2Type = std::array<float, 2>;
	using FloatType = float;

	using ValueType = std::variant<Vec4Type, Vec3Type, Vec2Type, FloatType>;

	namespace model
	{
		struct Node;

		

		struct OutputPin
		{
			Node* owner;
		};

		struct InputPin
		{
			core::u64 nodeId;
			core::u64 outputIndex;
		};

		struct Node
		{
			NodeType type;
			std::vector<InputPin> input;
			std::vector<ValueType> content;
			std::vector<OutputPin> output;
		};
	}

	namespace resolver
	{
		namespace glslGenerator
		{
		}

		struct ColorNodeResolver
		{
			static constexpr core::u32 inputCount = 0;
			static constexpr core::u32 outputCount = 1;
			static constexpr core::u32 contentCount = 1;

			std::array<std::string, 1> outputs = { std::string{"<content1>"}};
		};
	}

	

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

	struct InputPin
	{
		Pin pin;
		std::optional<ValueType> value{};
	};

	struct OutputPin
	{
		Pin pin;
		std::optional<ed::PinId> connection;
	};


	enum class TestSelectionEnum
	{
		tik,
		tak,
		boo,
		foo
	};

	struct TestNodeExtention
	{
		TestSelectionEnum selection;
	};

	struct Link
	{
		ed::LinkId id{};
		ed::PinId fromPinId{};
		ed::PinId toPinId{};
	};

	struct NoneExtension{};

	struct ColorSelectionNode
	{
		enum class ColorSpace
		{
			RGBA,
			HSV
		};

		ColorSelectionNode(core::u32 nodeId) : id{nodeId}{}
		
		ColorSpace selectedColorSpace{ColorSpace::RGBA};
		core::u32 id;
		Vec4Type color{};

		void draw()
		{
			const char* items[] = { "RGB", "HSV" };
			int selected = (int)selectedColorSpace;

			ImGui::PushID(id);
			ImGui::RadioButton(items[0], &selected, 0);
			ImGui::SameLine();
			ImGui::RadioButton(items[1], &selected, 1);

			

			auto flags = ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions |ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoDragDrop;

			switch (selectedColorSpace)
			{
			case toy::editor::materials::ColorSelectionNode::ColorSpace::RGBA:
				flags |= ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB;
				break;
			case toy::editor::materials::ColorSelectionNode::ColorSpace::HSV:
				flags |= ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_DisplayHSV;
				break;
			default:
				break;
			}

			ImGui::SetNextItemWidth(200);
			ImGui::ColorEdit4("", color.data(), flags);

			ImGui::PopID();

			selectedColorSpace = (ColorSpace)selected;
		}
	};

	using ExtenstionType = std::variant<NoneExtension, ColorSelectionNode>;

	struct Node
	{
		ed::NodeId id{};
		std::string title{};
		std::string description{};
		ImColor accentColor{};
		std::vector<Pin> inputPins{};
		std::vector<Pin> outputPins{};

		ExtenstionType extension{};

		NodeType type;
	};

	struct MaterialOutputNode
	{
		static auto create() -> Node
		{
			auto surface = Pin{};
			surface.id = ed::PinId{UIDGenerator::generate()};
			surface.name = "Surface";
			surface.accentColor = ImColor(0,150,0);
			surface.valueType = PinType::vector4Type;

			const auto nodeId = UIDGenerator::generate();

			auto node = Node{};
			node.id = ed::NodeId{nodeId};
			node.title = "Material Output";
			node.accentColor = ImColor(100,0,0);
			node.inputPins.push_back(surface);
			node.type = NodeType::materialOutput;

			return node;
		}

		static auto generateModel(const Node& node) -> model::Node
		{
			TOY_ASSERT(node.type == NodeType::materialOutput);
			TOY_ASSERT(node.inputPins.size() == 1);
			TOY_ASSERT(node.outputPins.size() == 0);
			auto model = model::Node{};

			auto input = model::InputPin{};

			input.nodeId = (const core::u64)node.inputPins[0].id;

			TOY_ASSERT(std::holds_alternative<NoneExtension>(node.extension));
			model.input.push_back(model::InputPin{});
			return model;
		}
	};

	struct ColorNode
	{
		static auto create() -> Node
		{
			auto color = Pin{};
			color.id = ed::PinId{UIDGenerator::generate()};
			color.name = "color";
			color.accentColor = ImColor(0,150,0);//<< vec2 type color
			color.valueType = PinType::vector4Type;
			
			const auto nodeId = UIDGenerator::generate();

			auto node = Node{};
			node.id = ed::NodeId{nodeId};
			node.title = "Color";
			node.accentColor = ImColor(100,0,0);
			node.outputPins.push_back(color);

			node.extension = ColorSelectionNode{nodeId};
			node.type = NodeType::color;
			return node;
		}

		static auto generateModel(const Node& node) -> model::Node
		{
			auto model = model::Node{};

			TOY_ASSERT(std::holds_alternative<ColorSelectionNode>(node.extension));
			auto colorExtension = std::get<ColorSelectionNode>(node.extension);
			model.content.push_back(colorExtension.color);
			model.output.push_back(model::OutputPin{});
			return model;
		}
	};
	

	auto createMaterialOutputNode() -> Node
	{
		auto surface = Pin{};
		surface.id = ed::PinId{UIDGenerator::generate()};
		surface.name = "Surface";
		surface.accentColor = ImColor(0,150,0);
		surface.valueType = PinType::vector4Type;

		const auto nodeId = UIDGenerator::generate();

		auto node = Node{};
		node.id = ed::NodeId{nodeId};
		node.title = "Material Output";
		node.accentColor = ImColor(100,0,0);
		node.inputPins.push_back(surface);

		return node;
	}

	//auto createMaterialOutputNode() -> Node
	//{
	//	auto surface = Pin{};
	//	surface.id = ed::PinId{UID::generate()};
	//	surface.name = "Surface";
	//	surface.color = ImColor(0,150,0);
	//	surface.type = PinType::shading;

	//	auto node = Node{};
	//	node.id = ed::NodeId{UID::generate()};
	//	node.name = "Material Output";
	//	node.color = ImColor(100,0,0);
	//	node.inputPins.push_back(surface);

	//	return node;
	//}

	//auto createTextureCoordinateNode() -> Node
	//{
	//	auto uv = Pin{};
	//	uv.id = ed::PinId{UID::generate()};
	//	uv.name = "uv";
	//	uv.color = ImColor(0,150,0);//<< vec2 type color
	//	uv.type = PinType::vector2;

	//	auto node = Node{};
	//	node.id = ed::NodeId{UID::generate()};
	//	node.name = "Texture Coordinate";
	//	node.color = ImColor(100,0,0);
	//	node.outputPins.push_back(uv);
	//	return node;
	//}

	//auto createShadeNode() -> Node
	//{
	//	auto color = Pin{};
	//	color.id = ed::PinId{UID::generate()};
	//	color.name = "color";
	//	color.color = ImColor(0,150,0);//<< vec2 type color
	//	color.type = PinType::vector3;

	//	auto surface = Pin{};
	//	surface.id = ed::PinId{UID::generate()};
	//	surface.name = "Surface";
	//	surface.color = ImColor(0,150,0);
	//	surface.type = PinType::shading;

	//	auto node = Node{};
	//	node.id = ed::NodeId{UID::generate()};
	//	node.name = "Texture Coordinate";
	//	node.color = ImColor(100,0,0);
	//	node.outputPins.push_back(surface);
	//	node.inputPins.push_back(color);
	//	return node;
	//}

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	struct MaterialGlslResolver
	{
		using ResolveResultType = std::string;

		inline static auto resolveVec4(const Vec4Type value) -> ResolveResultType
		{
			return std::format("vec4({}, {}, {}, {})", value[0], value[1], value[2], value[3]);
		}

		inline static auto resolveVec3(const Vec3Type value) -> ResolveResultType
		{
			return std::format("vec3({}, {}, {})", value[0], value[1], value[2]);
		}

		inline static auto resolveVec2(const Vec2Type value) -> ResolveResultType
		{
			return std::format("vec2({}, {})", value[0], value[1]);
		}

		inline static auto resolveFloat(const FloatType value) -> ResolveResultType
		{
			return std::format("{}", value);
		}

		inline static auto resloveValue(const ValueType value) -> ResolveResultType
		{
			return std::visit(
				overloaded
				{
		            [](Vec4Type arg) { return resolveVec4(arg); },
		            [](Vec3Type arg) { return resolveVec3(arg); },
		            [](Vec2Type arg) { return resolveVec2(arg); },
		            [](FloatType arg) { return resolveFloat(arg); }
				},
				value);
		}

		struct OutputPin;

		struct InputPin
		{
			bool isConnected{false};
			OutputPin* connectedPin{};
			std::optional<ValueType> optionalValue{};
		};

		struct OutputPin
		{
			
		};

		inline auto isConnected(const InputPin& pin) -> bool
		{
			return pin.isConnected;
		}

		auto resolveInputPin(const InputPin& pin) -> ResolveResultType
		{
			auto value = ResolveResultType{};
			if(isConnected(pin))
			{
				assert(pin.connectedPin);
				value = resolveOutputPin(*pin.connectedPin);
			}
			else
			{
				assert(pin.optionalValue.has_value());
				value = resloveValue(pin.optionalValue.value());
			}

			return value;
		}

		auto resolveOutputPin(const OutputPin& pin) -> ResolveResultType
		{
			return ResolveResultType{};
		}
	};

	auto drawExtention(ExtenstionType& extension) -> void
	{
		return std::visit(
				overloaded
				{
		            [](NoneExtension arg) { },
		            [](ColorSelectionNode& arg) { arg.draw(); }
				},
				extension);
	}

	struct MaterialEditor
	{
		auto loadMaterialFromFile(std::string path);
		auto saveMaterialToFile(std::string filename);

		//window size, position ect.
		auto drawNode(Node& node) -> void;
		auto drawMaterialEditor() -> void;
        auto initialize() -> void;

    private:

		ed::NodeId selectedNodeId{};
		ed::PinId selectedPinId{};
		ed::LinkId selectedLinkId{};

		ed::PinId pulledInputPin_{};
		ed::PinId pulledOutputPin_{};

        ed::EditorContext* context_;
        bool isFirstFrame_ = true;
		std::vector<Node> nodes_;
		std::vector<Link> links_;

		std::unordered_map<core::u32, model::Node> graphModel_;
	};

	auto MaterialEditor::drawNode(Node& node) -> void
	{
		ed::BeginNode(node.id);
				
	            ImGui::Text(node.title.c_str());
				
				drawExtention(node.extension);

				for(const auto& pin : node.outputPins)
				{
					ImGui::Text(pin.name.c_str());
					ImGui::SameLine();
					ed::BeginPin(pin.id, ed::PinKind::Output);
					const auto currentPosition = ImGui::GetCursorScreenPos();
					
					ImGui::Dummy(ImVec2(16,16));

					if(ImGui::IsItemHovered() || pin.id  == pulledOutputPin_ || pin.id == pulledInputPin_)
					{
						ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(currentPosition.x + 8,currentPosition.y + 8), 4, pin.accentColor, 24);
					}
					else
					{
						ImGui::GetWindowDrawList()->AddCircle(ImVec2(currentPosition.x + 8, currentPosition.y + 8), 5, pin.accentColor, 24, 2);
					}
		            ed::EndPin();
				}

				for(const auto& pin : node.inputPins)
				{
					ed::BeginPin(pin.id, ed::PinKind::Input);
					const auto currentPosition = ImGui::GetCursorScreenPos();
					ImGui::Dummy(ImVec2(16,16));
					if(ImGui::IsItemHovered() || pin.id == pulledOutputPin_ || pin.id == pulledInputPin_ )
					{
						ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(currentPosition.x + 8,currentPosition.y + 8), 4, pin.accentColor, 24);
					}
					else
					{
						ImGui::GetWindowDrawList()->AddCircle(ImVec2(currentPosition.x + 8,currentPosition.y + 8), 5, pin.accentColor, 24, 2);
					}
		            ed::EndPin();
					ImGui::SameLine();
		            ImGui::Text(pin.name.c_str());
				}
	            
	            ed::EndNode();
	}


	auto MaterialEditor::drawMaterialEditor() -> void
	{
		if (ImGui::Begin("Node Editor"))
        {
			static auto generatedCode = std::string{};

			if(ImGui::Button("generate"))
			{
				
				graphModel_.clear();

				for(const auto& node : nodes_)
				{
					if(node.type == NodeType::color)
					{
						const auto model = ColorNode::generateModel(node);
						graphModel_.insert(std::make_pair(UIDGenerator::generate(), model));
					}

					if(node.type == NodeType::materialOutput)
					{
						const auto model = MaterialOutputNode::generateModel(node);
						graphModel_.insert(std::make_pair(UIDGenerator::generate(), model));
					}
				}

				

				generatedCode = "!!!!";
			}

			ImGui::Text(generatedCode.c_str());

			ed::SetCurrentEditor(context_);

			auto selectedLinks = std::vector<ed::LinkId>{};
			auto selectedNodes = std::vector<ed::NodeId>{};

			const auto selectedObjectCount = ed::GetSelectedObjectCount();

			selectedLinks.resize(selectedObjectCount);
			selectedNodes.resize(selectedObjectCount);

			const auto selectedLinkCount = ed::GetSelectedLinks(selectedLinks.data(), selectedLinks.size());
			const auto selectedNodeCount = ed::GetSelectedNodes(selectedNodes.data(), selectedNodes.size());

			selectedLinks.resize(selectedLinkCount);
			selectedNodes.resize(selectedNodeCount);


			//Workaround: It fixes the bug with strange behaved clipping
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(5, 5),ImVec2(6, 6), IM_COL32(0, 0, 0, 1));

			ed::Begin("My Node Editor");
			auto cursorTopLeft = ImGui::GetCursorScreenPos();

			ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(0,0,0,0));

            for(auto& node : nodes_)
            {
	            drawNode(node);
            }
			ed::PopStyleColor();

			for(const auto& link : links_)
			{
				ed::Link(link.id, link.fromPinId, link.toPinId);
			}

	        if(ed::BeginCreate())
	        {
	            ed::PinId iPinId, oPinId;
				
	            if (ed::QueryNewLink(&iPinId, &oPinId))
	            {
					pulledOutputPin_ = oPinId;
					pulledInputPin_ = iPinId;
	                if (iPinId && oPinId) // both are valid, let's accept link
	                {
	                    // ed::AcceptNewItem() return true when user release mouse button.
	                    if (ed::AcceptNewItem())
	                    {
							const auto linkId = UIDGenerator::generate();
							links_.push_back(Link{linkId, iPinId, oPinId });
	                        // Draw new link.
	                        ed::Link(linkId, iPinId, oPinId);
	                    }
	                }
	            }
				else
				{
					pulledOutputPin_ = ed::PinId{};
					pulledInputPin_ = ed::PinId{};
				}
	        }
	        ed::EndCreate();

			if(ed::BeginDelete())
			{
				auto linkId = ed::LinkId{};
				while(ed::QueryDeletedLink(&linkId))
				{
					if(ed::AcceptDeletedItem())
					{
						std::erase_if(links_, 
							[&linkId](Link& link)
							{
								return link.id == linkId;
							}
						);
					}
				}

				auto nodeId = ed::NodeId{};
				while(ed::QueryDeletedNode(&nodeId))
				{
					if(ed::AcceptDeletedItem())
					{
						std::erase_if(nodes_, 
							[&nodeId](Node& node)
							{
								return node.id == nodeId;
							}
						);
					}
				}
			}
			ed::EndDelete();

			ImGui::SetCursorScreenPos(cursorTopLeft);

			auto openPopupPosition = ImGui::GetMousePos();
	        ed::Suspend();
	        if (ed::ShowNodeContextMenu(&selectedNodeId))
	        {
	            ImGui::OpenPopup("NodeContextMenu");
	        }
	        else if (ed::ShowPinContextMenu(&selectedPinId))
	        {
		        ImGui::OpenPopup("PinContextMenu");
	        }
	        else if (ed::ShowLinkContextMenu(&selectedLinkId))
	        {
		        ImGui::OpenPopup("LinkContextMenu");
	        }
	        else if (ed::ShowBackgroundContextMenu())
	        {
	            ImGui::OpenPopup("EditorContextMenu");
	        }

			if(ImGui::BeginPopup("NodeContextMenu"))
			{
				if (ImGui::MenuItem("Delete"))
				{
					ed::DeleteNode(selectedNodeId);
				}
				ImGui::EndPopup();
			}

			if(ImGui::BeginPopup("LinkContextMenu"))
			{
				if (ImGui::MenuItem("Delete"))
				{
					ed::DeleteLink(selectedLinkId);
				}
				ImGui::EndPopup();
			}

			if(ImGui::BeginPopup("EditorContextMenu"))
			{
				if(ImGui::MenuItem("Add Material Output"))
				{
					auto node = MaterialOutputNode::create();
					nodes_.push_back(node);
				}
				if(ImGui::MenuItem("Add Color Node"))
				{
					auto node = ColorNode::create();
					nodes_.push_back(node);
				}

				ImGui::EndPopup();
			}

			ed::Resume();

			ed::End();
            ed::SetCurrentEditor(nullptr);
        }
		ImGui::End();
        isFirstFrame_ = false;
	}

    auto MaterialEditor::initialize() -> void
	{
		ed::Config nodeConfig;
	    nodeConfig.SettingsFile = "Simple.json";
	    context_ = ed::CreateEditor(&nodeConfig);
	}
}