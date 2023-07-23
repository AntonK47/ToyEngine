#pragma once

#include <MaterialEditorNode.h>
#include <MaterialEditorGlslResolver.h>
#include <string>
#include <MaterialNodeRegistry.h>

namespace toy::editor
{
	struct ScalarNode final : public MaterialNode
	{
		ScalarNode();
		void drawNodeContent() override;
		std::string toString() override;

		struct State : NodeState
		{
			float scalar{ 1 };
			inline std::unique_ptr<NodeState> clone() override
			{
				return std::make_unique<State>(*this);
			}
		} state_{}, lastState_{};

		std::unique_ptr<NodeState> getStateCopy() override;
		void submitState() override;
		void setState(NodeState* state) override;
		//REGISTER_NODE("Input", "Scalar", ScalarNode)
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