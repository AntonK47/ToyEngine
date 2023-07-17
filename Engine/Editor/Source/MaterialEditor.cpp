#include "MaterialEditor.h"

inline auto toy::editor::MaterialEditor::drawNode(MaterialNode& node) -> void
{
	
	ed::PushStyleColor(ed::StyleColor_NodeBorder, node.headerColor);
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

	const auto radius = 2;
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
		const auto pinColor = getTypePinColor(pin.valueType);
		const auto borderPosition = ImVec2(headerMax.x + padding.x - border, currentPosition.y + 10);
		const auto stretch = isHovered ? 3 : 1;
		ed::PinRect(borderPosition + ImVec2(-radius, 0) * 2.0, borderPosition + ImVec2(radius, radius) * 2.0);
		ed::PinPivotRect(borderPosition + ImVec2(-radius, radius * 0.5), borderPosition + ImVec2(radius * 0.5, radius * 0.5));
		const auto center = (borderPosition + ImVec2(-radius, 0) + borderPosition + ImVec2(radius, radius)) * 0.5;
		ImGui::GetWindowDrawList()->AddCircleFilled(center - ImVec2(stretch, 0), radius, pinColor, 24);
		ImGui::GetWindowDrawList()->AddRectFilled(center - ImVec2(stretch, radius), center + ImVec2(radius + (isHovered || isNodeHovered || isNodeSelected ? border * 0.5 : 0) , radius), pinColor) ;

		ed::EndPin();
	}

	node.draw();

	for (const auto& pin : node.inputPins)
	{
		const auto pinHalfSize = ImVec2(4, 16);
		ed::BeginPin(pin.id, ed::PinKind::Input);
		const auto currentPosition = ImGui::GetCursorScreenPos();
		ImGui::Dummy(pinHalfSize * ImVec2(2, 1));
		ImGui::SameLine();
		const auto center = ImGui::GetItemRectSize() * ImVec2(0.5, 0.5) + currentPosition;
		const auto radius = ImGui::IsItemHovered() || pin.id == pulledOutputPin_ || pin.id == pulledInputPin_ ? 3 : 4;
		ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, getTypePinColor(pin.valueType), 24);
		ed::EndPin();
		ImGui::SameLine();
		ImGui::Text(pin.name.c_str());
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
				headerMax + ImVec2(halfBorderWidth + 8 - padding.y + 1, 0), node.headerColor,
				ed::GetStyle().NodeRounding - 1, ImDrawFlags_RoundCornersTop);
		}
	}
	node.deferredDraw();
}

inline void toy::editor::MaterialEditor::onDrawGui()
{
	
	static auto generatedCode = std::string{};

	if (ImGui::Button("generate"))
	{

		graphModel_.clear();
		generatedCode = "!!!!";
	}
	
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
		ed::Link(link.id, link.fromPinId, link.toPinId);
	}

	if (ed::BeginCreate())
	{
		ed::PinId iPinId, oPinId;

		if (ed::QueryNewLink(&iPinId, &oPinId))
		{
			pulledOutputPin_ = oPinId;
			pulledInputPin_ = iPinId;
			if (iPinId && oPinId)
			{
				if (ed::AcceptNewItem())
				{
					const auto linkId = core::UIDGenerator::generate();
					links_.push_back(Link{ linkId, iPinId, oPinId });
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

	if (ed::BeginDelete())
	{
		auto linkId = ed::LinkId{};
		while (ed::QueryDeletedLink(&linkId))
		{
			if (ed::AcceptDeletedItem())
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
		while (ed::QueryDeletedNode(&nodeId))
		{
			if (ed::AcceptDeletedItem())
			{
				std::erase_if(nodes_,
					[&nodeId](std::unique_ptr<MaterialNode>& node)
					{
						return node->id == nodeId;
					}
				);
			}
		}
	}
	ed::EndDelete();

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
			/*auto node = MaterialOutputNode::create();
			nodes_.push_back(node);*/
		}
		if (ImGui::BeginMenu("Input"))
		{

			if (ImGui::MenuItem("Color"))
			{
				/*if(ImGui::M)*/
				auto node = std::make_unique<ColorNode>();
				nodes_.push_back(std::move(node));
			}
			if (ImGui::MenuItem("Scalar"))
			{
				auto node = std::make_unique<ScalarNode>();
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

inline void toy::editor::MaterialEditor::initialize()
{
	ed::Config nodeConfig{};
	nodeConfig.SettingsFile = nullptr;// "Simple.json";
	context_ = ed::CreateEditor(&nodeConfig);
}

inline void toy::editor::MaterialEditor::deinitialize()
{
	ed::DestroyEditor(context_);
}