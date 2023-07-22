#pragma once

#include <MaterialEditorNode.h>
#include <MaterialEditorGlslResolver.h>
#include <unordered_map>

namespace toy::editor
{
	struct ColorNode final : public MaterialNode
	{
		enum class ColorSpace
		{
			RGBA,
			HSV
		};

		ColorNode();

		ColorSpace selectedColorSpace{ ColorSpace::RGBA };
		Vec4Type color{ 0.0f, 0.0f, 0.0f, 1.0f };
		bool openPickerPopup{ false };
		bool openColorSpaceDropDown{ false };

		void drawNodeContent() override;
		void deferredDraw() override;

		std::unique_ptr<NodeState> getStateCopy() override;
		void submitState() override;
		void setState(NodeState* state) override;
	};
}

namespace toy::editor::resolver::glsl
{
	struct ColorNodeGlslResolver final : public MaterialNodeGlslResolver
	{
		ColorNodeGlslResolver(MaterialNodeGlslResolver::MaterialEditorResolverType* resolver, MaterialNode* node)
			: MaterialNodeGlslResolver{ resolver, node } {}

		using NodeType = ColorNode;
		auto resolve(core::u8 outputPinIndex = 0) -> PinResolveResult* override;

	};
}