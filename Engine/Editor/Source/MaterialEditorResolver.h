#pragma once
#include <Core.h>
#include <memory>

namespace toy::editor
{
	struct MaterialNode;
	struct Link;
}

namespace toy::editor::resolver
{
	struct MaterialModel
	{
		virtual auto nodes() -> std::vector<std::unique_ptr<MaterialNode>>& = 0;
		virtual auto links() -> std::vector<std::unique_ptr<Link>>& = 0;
	};

	struct PinResolveResult
	{
		virtual ~PinResolveResult() {}
	};

	struct MaterialNodeResolver
	{
		MaterialNode* node;

		MaterialNodeResolver(MaterialNode* node) : node(node){}

		auto virtual resolve(core::u8 outputPinIndex) -> PinResolveResult* = 0;
		virtual ~MaterialNodeResolver() {};
	};

	struct ResolveResult
	{
		virtual ~ResolveResult() {}
	};

	struct MaterialEditorResolver
	{
		auto solve(MaterialModel& model) -> void;
		//virtual auto solve(const std::vector<Pin>& pins) -> std::vector<PinResolveResult> = 0;
		virtual auto reset() -> void {};
		virtual ~MaterialEditorResolver() {};
	};
}