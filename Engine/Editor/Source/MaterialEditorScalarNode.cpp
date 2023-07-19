#include <MaterialEditorGlslResolver.h>
#include <MaterialEditorScalarNode.h>
#include <MaterialEditor.h>
#include <format>

using namespace toy::editor;
using namespace toy::editor::resolver;
using namespace toy::editor::resolver::glsl;

inline ScalarNode::ScalarNode()
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

inline void ScalarNode::draw()
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(getTypePinColor(PinType::scalarType)));
	ImGui::SetNextItemWidth(width);
	ImGui::PushID(id.Get());
	ImGui::DragFloat("", &scalar, 0.1f, 0.0f, 0.0f, "%0.1f");
	ImGui::PopID();
	ImGui::PopStyleColor();
}

inline auto ScalarNodeGlslResolver::resolve(core::u8 outputPinIndex) -> PinResolveResult*
{
	const auto scalarNode = reinterpret_cast<const NodeType*>(node);

	if (cachedResults.contains(outputPinIndex))
	{
		return cachedResults[outputPinIndex].get();
	}

	auto result = std::make_unique<MaterialEditorGlslResolver::PinResolveResultType>();
	if (node->outputPins[outputPinIndex].referenceCount > 1)
	{
		auto var = resolver->generateVariableName();
		resolver->localDeclarationBlock.push_back(std::format("float {} = {};", var, scalarNode->scalar));

		result->value = var;
	}
	else
	{
		result->value = std::to_string(scalarNode->scalar);
	}

	cachedResults[outputPinIndex] = std::move(result);
	return cachedResults[outputPinIndex].get();
}
