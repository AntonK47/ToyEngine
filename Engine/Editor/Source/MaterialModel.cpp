#include <MaterialModel.h>
#include <MaterialEditorNode.h>

using namespace toy::editor;

MaterialNode* MaterialModel::findNode(const ed::NodeId id)
{
	for (auto& node : nodes())
	{
		if (node->id == id)
			return node.get();
	}
	TOY_ASSERT(true);
}

OutputPin* MaterialModel::findOutputPin(const ed::PinId id)
{
	for (const auto& node : nodes())
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

InputPin* MaterialModel::findInputPin(const ed::PinId id)
{
	for (const auto& node : nodes())
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