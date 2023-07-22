#include "MaterialEditorScalarNode.h"

#include <MaterialEditorGlslResolver.h>
#include <MaterialEditor.h>
#include <format>

using namespace toy::editor;
using namespace toy::editor::resolver;
using namespace toy::editor::resolver::glsl;

inline ScalarNode::ScalarNode()
{
	auto scalarPin = OutputPin{};
	scalarPin.id = ed::PinId{ core::UIDGenerator::generate() };
	scalarPin.name = "scalar";
	scalarPin.type = PinType::scalarType;
	scalarPin.nodeOwner = this;

	const auto nodeId = core::UIDGenerator::generate();

	id = ed::NodeId{ nodeId };
	title = "Scalar";
	outputPins.push_back(scalarPin);
	nodeColor = getTypePinColor(PinType::scalarType);
}

inline void ScalarNode::drawNodeContent()
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::scalarType)));
	ImGui::SetNextItemWidth(width);
	ImGui::PushID(id.Get());
	ImGui::DragFloat("", &state_.scalar, 0.1f, 0.0f, 0.0f, "%0.1f");
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		hasStateChanged = true;
	}
	ImGui::PopID();
	ImGui::PopStyleColor();
}

std::string ScalarNode::toString()
{
	return std::format("ScalarNode [{}]", id.Get());
}

std::unique_ptr<NodeState> ScalarNode::getStateCopy()
{
	return std::make_unique<ScalarNode::State>(lastState_);
}

void ScalarNode::submitState()
{
	lastState_ = state_;
}

void ScalarNode::setState(NodeState* state)
{
	state_.scalar = dynamic_cast<ScalarNode::State*>(state)->scalar;
}

inline auto ScalarNodeGlslResolver::resolve(core::u8 outputPinIndex) -> PinResolveResult*
{
	const auto scalarNode = reinterpret_cast<const NodeType*>(node);

	if (cachedResults.contains(outputPinIndex))
	{
		return cachedResults[outputPinIndex].get();
	}

	auto result = std::make_unique<MaterialEditorGlslResolver::PinResolveResultType>();
	if (node->outputPins[outputPinIndex].connectedLinksCount > 1)
	{
		auto var = resolver->generateVariableName();
		resolver->localDeclarationBlock.push_back(std::format("float {} = {};", var, scalarNode->state_.scalar));

		result->value = var;
	}
	else
	{
		result->value = std::to_string(scalarNode->state_.scalar);
	}

	cachedResults[outputPinIndex] = std::move(result);
	return cachedResults[outputPinIndex].get();
}
