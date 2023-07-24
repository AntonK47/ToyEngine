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

void MoveNodeAction::redo()
{
	auto node = model->findNode(nodeId);
	if (node)
	{
		node->position = newPosition;
		ed::SetNodePosition(node->id, node->position);
	}
}

void MoveNodeAction::undo()
{
	auto node = model->findNode(nodeId);
	if (node)
	{
		node->position = lastPosition;
		ed::SetNodePosition(node->id, node->position);
	}
}

std::string MoveNodeAction::toString()
{
	return std::format("Node [{}] moved to [{}, {}]", nodeId.Get(), newPosition.x, newPosition.y);
}

void NodeStateChangeAction::redo()
{
	auto node = model->findNode(nodeId);
	if (node)
	{
		node->setState(newState.get());
	}
}

void NodeStateChangeAction::undo()
{
	auto node = model->findNode(nodeId);
	if (node)
	{
		node->setState(lastState.get());
	}
}

std::string NodeStateChangeAction::toString()
{
	return std::format("Node [{}] changes values", nodeId.Get());
}