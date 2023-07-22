#include "MaterialEditorNode.h"
#include "MaterialEditorResolver.h"

#include <format>

using namespace toy::editor;

auto MaterialNode::resolve(core::u8 outputPinIndex) -> resolver::PinResolveResult*
{
	if (nodeResolver != nullptr)
	{
		return nodeResolver->resolve(outputPinIndex);
	}
	return nullptr;
}

toy::editor::MaterialNode::~MaterialNode()
{
	

}

inline Link::~Link()
{
	TOY_ASSERT(fromPin->connectedLinksCount > 0);
	TOY_ASSERT(toPin->connectedLink);
	fromPin->connectedLinksCount--;
	toPin->connectedLink = nullptr;
	toPin->nodeOwner->notifyInputPinConnection();
}

void toy::editor::MoveNodeAction::redo()
{
	node->position = newPosition;
	ed::SetNodePosition(node->id, node->position);
}

void toy::editor::MoveNodeAction::undo()
{
	node->position = lastPosition;
	ed::SetNodePosition(node->id, node->position);
}

std::string toy::editor::MoveNodeAction::toString()
{
	return std::format("{} moved to [{}, {}]", node->toString(), newPosition.x, newPosition.y);
}
