#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "MaterialEditorResolver.h"

namespace toy::editor::resolver::glsl
{
	struct GlslPinResolveResult : public PinResolveResult
	{
		std::string value{};
	};

	struct MaterialEditorGlslResolver : public MaterialEditorResolver
	{
		using PinResolveResultType = GlslPinResolveResult;

		std::vector<std::string> localDeclarationBlock{};
		int uid{ 0 };

		auto reset() -> void override
		{
		}

		auto generateVariableName() -> std::string;
	};

	struct MaterialNodeGlslResolver : public MaterialNodeResolver
	{
		using MaterialEditorResolverType = MaterialEditorGlslResolver;

		MaterialEditorResolverType* resolver;
		std::unordered_map<core::u8, std::unique_ptr<PinResolveResult>> cachedResults;
		bool isResolved{ false };

		MaterialNodeGlslResolver(MaterialEditorResolverType* resolver, MaterialNode* node)
			: MaterialNodeResolver{ node }, resolver{ resolver } {}
	};
}