#include "MaterialEditorNode.h"
#include "MaterialEditorResolver.h"

using namespace toy::editor;

auto MaterialNode::resolve(core::u8 outputPinIndex) -> resolver::PinResolveResult*
{
	if (nodeResolver != nullptr)
	{
		return nodeResolver->resolve(outputPinIndex);
	}
	return nullptr;
}