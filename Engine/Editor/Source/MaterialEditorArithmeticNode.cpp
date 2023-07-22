#include "MaterialEditorArithmeticNode.h"

#include <MaterialEditorGlslResolver.h>
#include <MaterialEditorNode.h>
#include <MaterialEditor.h>

using namespace toy::editor;
using namespace toy::editor::resolver;
using namespace toy::editor::resolver::glsl;

ArithmeticNode::ArithmeticNode()
{
	auto result = OutputPin{};
	result.id = ed::PinId{ core::UIDGenerator::generate() };
	result.name = "";
	result.type = PinType::scalarType;
	result.nodeOwner = this;

	auto input1 = InputPin{};
	input1.id = ed::PinId{ core::UIDGenerator::generate() };
	input1.name = "a";
	input1.type = PinType::scalarType;
	input1.nodeOwner = this;

	auto input2 = InputPin{};
	input2.id = ed::PinId{ core::UIDGenerator::generate() };
	input2.name = "b";
	input2.type = PinType::scalarType;
	input2.nodeOwner = this;

	const auto nodeId = core::UIDGenerator::generate();

	id = ed::NodeId{ nodeId };
	title = "Arithmetic";
	outputPins.push_back(result);
	inputPins.push_back(input1);
	inputPins.push_back(input2);
	nodeColor = getTypePinColor(PinType::scalarType);
}

void ArithmeticNode::drawNodeContent()
{
}

void ArithmeticNode::drawPinContent(float maxWidth, core::u8 pinIndex)
{
	switch (pinIndex)
	{
	case 0:
		if (valueA.has_value())
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::scalarType)));
			ImGui::SetNextItemWidth(maxWidth);
			ImGui::PushID("##valueA");
			ImGui::DragFloat("", &valueA.value(), 0.1f, 0.0f, 0.0f, "%0.1f");
			ImGui::PopID();
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::Text("value");
		}

		break;
	case 1:
		if (valueB.has_value())
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::scalarType)));
			ImGui::SetNextItemWidth(maxWidth);
			ImGui::PushID("##valueB");
			ImGui::DragFloat("", &valueB.value(), 0.1f, 0.0f, 0.0f, "%0.1f");
			ImGui::PopID();
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::Text("value");
		}
		break;
	default:
		ImGui::Text("value");
		break;
	}
}

void ArithmeticNode::notifyInputPinConnection()
{
	if (inputPins[0].connectedLink != nullptr)
	{
		valueA = std::nullopt;
	}
	else
	{
		valueA = 0.0f;
	}
	if (inputPins[1].connectedLink != nullptr)
	{
		valueB = std::nullopt;
	}
	else
	{
		valueB = 0.0f;
	}
}

std::string toy::editor::ArithmeticNode::toString()
{
	return std::format("ArithmeticNode [{}]", id.Get());
}

std::unique_ptr<NodeState> toy::editor::ArithmeticNode::getStateCopy()
{
	return std::unique_ptr<NodeState>();
}

void toy::editor::ArithmeticNode::submitState()
{
}

void toy::editor::ArithmeticNode::setState(NodeState* state)
{
}
