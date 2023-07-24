#include "MaterialEditorResolver.h"

#include <MaterialEditorNode.h>
#include <MaterialModel.h>
#include <memory>
#include <Core.h>
#include <unordered_map>

using namespace toy::editor;
using namespace toy::editor::resolver;

auto MaterialEditorResolver::solve(MaterialModel& model) -> void
{
	auto pinsRefs = std::unordered_map<std::uintptr_t, core::u8>{};
	for (const auto& link : model.links())
	{
		if (pinsRefs.contains(link->fromPin->id.Get()))
		{
			pinsRefs[link->fromPin->id.Get()]++;
		}
		else
		{
			pinsRefs[link->fromPin->id.Get()] = 1;
		}

		if (pinsRefs.contains(link->toPin->id.Get()))
		{
			pinsRefs[link->toPin->id.Get()]++;
		}
		else
		{
			pinsRefs[link->toPin->id.Get()] = 1;
		}
	}

	for (const auto& n : model.nodes())
	{
		for (auto& pin : n->outputPins)
		{
			if (pinsRefs.contains(pin.id.Get()))
			{
				//pin.referenceCount = pinsRefs[pin.id.Get()];
			}
		}
	}

	//select start pins
	for (const auto& n : model.nodes())
	{

	}

	/*for (const auto& n : model.nodes())
	{
		GlslPinResolveResult* result = dynamic_cast<GlslPinResolveResult*>(n->resolve(0).get());
		if (result)
			LOG(WARNING) << result->value;
	}


	MaterialEditorGlslResolver* r = dynamic_cast<MaterialEditorGlslResolver*>(this);
	for (const auto& rr : r->localDeclarationBlock)
	{
		LOG(WARNING) << rr;
	}*/
}