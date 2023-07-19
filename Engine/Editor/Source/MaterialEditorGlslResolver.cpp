#include "MaterialEditorGlslResolver.h"

#include <format>

using namespace toy::editor::resolver::glsl;

auto MaterialEditorGlslResolver::generateVariableName() -> std::string
{
	return std::format("_a{}", uid++);
}