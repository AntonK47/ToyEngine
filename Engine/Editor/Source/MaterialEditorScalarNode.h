#pragma once

#include <MaterialEditorNode.h>
#include <MaterialEditorGlslResolver.h>
#include <unordered_map>

namespace toy::editor
{
	struct ScalarNode final : public MaterialNode
	{
		ScalarNode();
		void draw() override;

		float scalar{ 1 };
	};
}

namespace toy::editor::resolver::glsl
{
	struct ScalarNodeGlslResolver final: public MaterialNodeGlslResolver
	{
		ScalarNodeGlslResolver(MaterialNodeGlslResolver::MaterialEditorResolverType* resolver, MaterialNode* node)
			: MaterialNodeGlslResolver{ resolver, node }{}

		using NodeType = ScalarNode;
		auto resolve(core::u8 outputPinIndex = 0) -> PinResolveResult* override;

	};
}