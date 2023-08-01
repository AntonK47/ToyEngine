#pragma once

#include <AssetDatabase.h>

namespace toy::editor::widget
{
	auto assetDropTarget(const char* label, const std::string_view AssetObjectType, AssetEntry* entry, const AssetDatabase& db) -> bool;
}