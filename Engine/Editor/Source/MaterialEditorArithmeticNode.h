#pragma once

#include <MaterialEditorNode.h>
#include <MaterialEditorGlslResolver.h>
#include <unordered_map>

namespace toy::editor
{
	struct ArithmeticNode final : public MaterialNode
	{
		ArithmeticNode();
		void drawNodeContent() override;
		void drawPinContent(float maxWidth, core::u8 pinIndex) override;
		void notifyInputPinConnection() override;
		std::string toString() override;

		std::unique_ptr<NodeState> getStateCopy() override;
		void submitState() override;
		void setState(NodeState* state) override;

		std::optional<float> valueA{0.0f};
		std::optional<float> valueB{0.0f};
	};
}

namespace toy::editor::resolver::glsl
{
	struct ArithmeticNodeNodeGlslResolver final: public MaterialNodeGlslResolver
	{
		ArithmeticNodeNodeGlslResolver(MaterialNodeGlslResolver::MaterialEditorResolverType* resolver, MaterialNode* node)
			: MaterialNodeGlslResolver{ resolver, node }{}

		using NodeType = ArithmeticNode;
		//auto resolve(core::u8 outputPinIndex = 0) -> PinResolveResult* override;

	};
}