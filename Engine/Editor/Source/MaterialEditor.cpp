#include "MaterialEditor.h"

#include <MaterialEditorNode.h>
#include <Logger.h>
#include <MaterialEditorScalarNode.h>
#include <MaterialEditorColorNode.h>
#include <MaterialEditorArithmeticNode.h>
#include <MaterialEditorGlslResolver.h>

#include <stack>
#include <memory>

using namespace toy::editor;

MaterialNode* toy::editor::MaterialEditor::findNode(const ed::NodeId id)
{
	for (auto& node : nodes_)
	{
		if (node->id == id)
			return node.get();
	}
	TOY_ASSERT(true);
}

inline auto MaterialEditor::drawNode(MaterialNode& node) -> void
{
	
	ed::PushStyleColor(ed::StyleColor_NodeBorder, node.nodeColor);
	ed::BeginNode(node.id);

	auto& style = ed::GetStyle();
	const auto headerHeight = ImGui::GetFontSize();
	auto pos = ImGui::GetCursorPos();
	ImGui::Dummy(ImVec2(node.width, headerHeight));
	const auto headerMin = ImGui::GetItemRectMin();
	const auto headerMax = ImGui::GetItemRectMax();
	ImGui::SameLine();
	ImGui::SetCursorPos(pos);
	ImGui::Text(node.title.c_str());

	const auto radius = 3;
	const auto padding = style.NodePadding;
	const auto border = style.NodeBorderWidth;
	const auto isNodeHovered = ed::GetHoveredNode() == node.id;
	const auto isNodeSelected = ed::IsNodeSelected(node.id);
	for (const auto& pin : node.outputPins)
	{
		const auto textSize = ImGui::CalcTextSize(pin.name.c_str());
		const auto pinHalfSize = ImVec2(4, 16);
		ImGui::Dummy(ImVec2(0, 1));
		ImGui::SameLine(node.width - textSize.x - padding.x - border);
		ImGui::Text(pin.name.c_str());
		ImGui::SameLine();
		
		ed::BeginPin(pin.id, ed::PinKind::Output);
		const auto currentPosition = ImGui::GetCursorScreenPos();
		const auto isHovered = ed::GetHoveredPin() == pin.id;
		const auto pinColor = getTypePinColor(pin.type);
		const auto borderPosition = ImVec2(headerMax.x + padding.x - border, currentPosition.y + 10);
		const auto stretch = isHovered ? 5 : 2;
		ed::PinRect(borderPosition + ImVec2(-radius, 0) * 2.0, borderPosition + ImVec2(radius, radius) * 2.0);
		ed::PinPivotRect(borderPosition + ImVec2(-radius, radius * 0.5), borderPosition + ImVec2(radius * 0.5, radius * 0.5));
		const auto center = (borderPosition + ImVec2(-radius, 0) + borderPosition + ImVec2(radius, radius)) * 0.5;
		ImGui::GetWindowDrawList()->AddCircleFilled(center - ImVec2(stretch, 0), radius, pinColor, 24);
		ImGui::GetWindowDrawList()->AddCircleFilled(center + ImVec2(radius + (isHovered || isNodeHovered || isNodeSelected ? 0 : -border), 0), radius, pinColor, 24);
		ImGui::GetWindowDrawList()->AddRectFilled(center - ImVec2(stretch, radius), center + ImVec2(radius + (isHovered || isNodeHovered || isNodeSelected ? 0 : -border) , radius), pinColor) ;

		ed::EndPin();
	}
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(node.nodeColor));
	node.drawNodeContent();
	ImGui::PopStyleColor();

	auto i = core::u8{};
	for (const auto& pin : node.inputPins)
	{
		ed::BeginPin(pin.id, ed::PinKind::Input);
		auto currentPosition = ImGui::GetCursorScreenPos();
		const auto isHovered = ed::GetHoveredPin() == pin.id;
		const auto pinColor = getTypePinColor(pin.type);
		const auto borderPosition = ImVec2(currentPosition.x - padding.x - border, currentPosition.y + 10);
		const auto stretch = isHovered ? 5 : 2;
		ed::PinRect(borderPosition + ImVec2(-radius, 0) * 2.0, borderPosition + ImVec2(radius, radius) * 2.0);
		ed::PinPivotRect(borderPosition + ImVec2(-radius, radius * 0.5), borderPosition + ImVec2(radius * 0.5, radius * 0.5));
		const auto center = (borderPosition + ImVec2(-radius, 0) + borderPosition + ImVec2(radius, radius)) * 0.5;
		ImGui::GetWindowDrawList()->AddCircleFilled(center - ImVec2(stretch, 0), radius, pinColor, 24);
		ImGui::GetWindowDrawList()->AddCircleFilled(center + ImVec2(radius + (isHovered || isNodeHovered || isNodeSelected ? 0 : -border), 0), radius, pinColor, 24);
		ImGui::GetWindowDrawList()->AddRectFilled(center - ImVec2(stretch, radius), center + ImVec2(radius + (isHovered || isNodeHovered || isNodeSelected ? 0 : -border), radius), pinColor);
		ed::EndPin();
		ImGui::SameLine();
		currentPosition = ImGui::GetCursorScreenPos();
		const auto maxWidth = headerMax.x - border - currentPosition.x;
		node.drawPinContent(maxWidth, i++);
		//ImGui::Text(pin.name.c_str());
	}

	ed::EndNode();
	ed::PopStyleColor();

	if (ImGui::IsItemVisible())
	{
		auto drawList = ed::GetNodeBackgroundDrawList(node.id);

		const auto halfBorderWidth = ed::GetStyle().NodeBorderWidth * 0.5;
		const auto padding = ImGui::GetStyle().FramePadding;
		
		if ((headerMax.x > headerMin.x) && (headerMax.y > headerMin.y))
		{
			drawList->AddRectFilled(headerMin - ImVec2(8 + halfBorderWidth - padding.x + 1, 8 + halfBorderWidth - padding.x + 1),
				headerMax + ImVec2(halfBorderWidth + 8 - padding.y + 1, 0), node.nodeColor,
				ed::GetStyle().NodeRounding - 1, ImDrawFlags_RoundCornersTop);
		}
	}
	node.deferredDraw();

	if (node.hasStateChanged)
	{
		auto lastState = node.getStateCopy();
		node.submitState();
		auto newState = node.getStateCopy();
		pushAction(std::make_unique<NodeStateChangeAction>(&node, std::move(lastState), std::move(newState)));
		node.hasStateChanged = false;
	}
}

inline void MaterialEditor::onDrawGui()
{
	auto generatedCode = std::string{};

	if (ImGui::Button("generate"))
	{
		generatedCode = "!!!!";

		resolver_->solve(*this);
	}

	ImGui::Text(generatedCode.c_str());
	

	ed::SetCurrentEditor(context_);
	ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(0, 0, 0, 0));
	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(0.0f, 0.0f, 0.0f, 1.0f));
	ed::PushStyleVar(ed::StyleVar_NodeRounding, 4);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	
	ed::Begin("My Node Editor");
	

	for (auto& node : nodes_)
	{
		auto& n = *node;
		drawNode(n);
	}
	
	for (const auto& link : links_)
	{
		ed::Link(link->id, link->fromPinId, link->toPinId, link->color, link->thickness);
	}

	if (ed::BeginCreate())
	{
		ed::PinId iPinId, oPinId;

		if (ed::QueryNewLink(&iPinId, &oPinId))
		{
			
			if (iPinId && oPinId)
			{
				auto inputPin = findInputPin(iPinId);

				if (inputPin)
				{
					ed::BreakLinks(iPinId);
				}

				auto outputPin = findOutputPin(oPinId);

				if (!inputPin || !outputPin)
				{
					inputPin = findInputPin(oPinId);
					outputPin = findOutputPin(iPinId);
				}


				auto isRejected = false;

				isRejected = !inputPin
					|| !outputPin
					|| (!inputPin->acceptedTypes.empty() && std::find(
						std::begin(inputPin->acceptedTypes),
						std::end(inputPin->acceptedTypes),
						outputPin->type) == std::end(inputPin->acceptedTypes))
					|| (inputPin->acceptedTypes.empty() && inputPin->type != outputPin->type)
					|| inputPin->nodeOwner == outputPin->nodeOwner;


				if (isRejected)
				{
					ed::RejectNewItem(ImVec4(255, 0, 0, 255), 2.0f);
				}
				else
				{
					if (ed::AcceptNewItem())
					{
						const auto linkId = core::UIDGenerator::generate();

						auto link = std::make_unique<Link>(linkId,outputPin->id, inputPin->id, outputPin, inputPin);
						inputPin->connectedLink = link.get();
						inputPin->nodeOwner->notifyInputPinConnection();

						const auto hasCircle = hasAnyCircle(outputPin->nodeOwner);
						if (hasCircle)
						{
							link->color = ImColor(255, 0, 0, 255);
							link->thickness = 4.0f;
						}

						outputPin->connectedLinksCount++;
						ed::Link(link->id, link->fromPinId, link->toPinId, link->color, link->thickness);
						links_.push_back(std::move(link));
					}
				}
			}
		}
	}
	ed::EndCreate();

	if (ed::BeginDelete())
	{
		auto linkId = ed::LinkId{};
		while (ed::QueryDeletedLink(&linkId))
		{
			if (ed::AcceptDeletedItem())
			{
				std::erase_if(links_,
					[&linkId](std::unique_ptr<Link>& link)
					{
						return link->id == linkId;
					}
				);
			}
		}

		auto nodeId = ed::NodeId{};
		while (ed::QueryDeletedNode(&nodeId))
		{
			if (ed::AcceptDeletedItem())
			{
				std::erase_if(nodes_,
					[&](std::unique_ptr<MaterialNode>& node)
					{
						if (node->id == nodeId)
						{
							for (const auto& pin : node->inputPins)
							{
								std::erase_if(links_,
									[&pin](std::unique_ptr<Link>& link)
									{
										return link->toPinId == pin.id;
									}
								);
							}

							for (const auto& pin : node->outputPins)
							{
								std::erase_if(links_,
									[&pin](std::unique_ptr<Link>& link)
									{
										return link->fromPinId == pin.id;
									}
								);
							}
							return true;
						}
						return false;
					}
				);


				
			}
		}
	}
	ed::EndDelete();

	
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
		ed::Resume();
		openPopupPosition_ = ImGui::GetMousePos();
		ed::Suspend();
	}

	if (ImGui::BeginPopup("NodeContextMenu"))
	{
		if (ImGui::MenuItem("Delete"))
		{
			ed::DeleteNode(selectedNodeId);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("LinkContextMenu"))
	{
		if (ImGui::MenuItem("Delete"))
		{
			ed::DeleteLink(selectedLinkId);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("EditorContextMenu"))
	{
		if (ImGui::MenuItem("Add Material Output"))
		{
		}
		if (ImGui::BeginMenu("Input"))
		{
			
			if (ImGui::MenuItem("Color"))
			{
				auto node = std::make_unique<ColorNode>();
				ed::SetNodePosition(node->id, openPopupPosition_);
				node->position = ed::GetNodePosition(node->id);
				nodes_.push_back(std::move(node));
			}
			if (ImGui::MenuItem("Scalar"))
			{
				auto node = std::make_unique<ScalarNode>();
				ed::SetNodePosition(node->id, openPopupPosition_);
				node->position = ed::GetNodePosition(node->id);
				//node->nodeResolver = std::make_unique<resolver::ScalarNodeGlslResolver>(resolver_.get());
				nodes_.push_back(std::move(node));
			}
			if (ImGui::MenuItem("Math"))
			{
				auto node = std::make_unique<ArithmeticNode>();
				ed::SetNodePosition(node->id, openPopupPosition_);
				node->position = ed::GetNodePosition(node->id);
				//node->nodeResolver = std::make_unique<resolver::ScalarNodeGlslResolver>(resolver_.get());
				nodes_.push_back(std::move(node));
			}
			ImGui::EndMenu();
		}
		

		ImGui::EndPopup();
	}
	ed::Resume();

	ed::End();
	ImGui::PopStyleVar(2);
	ed::PopStyleVar();
	ed::PopStyleColor(2);
}

OutputPin* MaterialEditor::findOutputPin(const ed::PinId id)
{
	for (const auto& node : nodes_)
	{
		for (auto& pin : node->outputPins)
		{
			if (pin.id == id)
			{
				return &pin;
			}
		}
	}
	return nullptr;
}

InputPin* MaterialEditor::findInputPin(const ed::PinId id)
{
	for (const auto& node : nodes_)
	{
		for (auto& pin : node->inputPins)
		{
			if (pin.id == id)
			{
				return &pin;
			}
		}
	}
	return nullptr;
}

bool toy::editor::MaterialEditor::hasAnyCircle(MaterialNode* root)
{
	auto visitedNodesCount = std::unordered_map<MaterialNode*, core::u8>{};

	auto visitedNodes = std::stack<MaterialNode*>{};
	visitedNodes.push(root);

	while (!visitedNodes.empty())
	{
		auto node = visitedNodes.top();
		visitedNodes.pop();
		visitedNodesCount[node]++;
		if (visitedNodesCount[node] > 1)
		{
			return true;
		}

		for (const auto& input : node->inputPins)
		{
			if (input.connectedLink)
			{
				visitedNodes.push(input.connectedLink->fromPin->nodeOwner);
			}
		}
	}
	return false;
}

inline void MaterialEditor::initialize()
{
	
	ed::Config config{};
	config.SettingsFile = nullptr;// "Simple.json";
	config.UserPointer = this;
	config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		{
			auto editor = (MaterialEditor*)userPointer;
			if ((reason & ed::SaveReasonFlags::Position) == ed::SaveReasonFlags::Position
				&& (reason & ed::SaveReasonFlags::AddNode) != ed::SaveReasonFlags::AddNode)
			{
				/*auto s = std::string{};

				s.assign(data, size);*/
				const auto position = ed::GetNodePosition(nodeId);
				auto node = editor->findNode(nodeId);

				if (node->position.x != position.x && node->position.y != position.y)
				{
					auto moveAction = std::make_unique<MoveNodeAction>(node, node->position, position);
					node->position = position;
					editor->pushAction(std::move(moveAction));

					LOG(WARNING) << "position changed!" << std::to_string(nodeId.Get()) << ": [" << position.x << ", " << position.y << "]";
				}
				
			}
			
			return true;
		};


	config.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		{
			auto s = std::string{};

			s.assign(data, size);

			LOG(WARNING) << s;
			return true;
		};
	context_ = ed::CreateEditor(&config);



	resolver_ = std::make_unique<resolver::glsl::MaterialEditorGlslResolver>();
}

inline void MaterialEditor::deinitialize()
{
	ed::DestroyEditor(context_);
}