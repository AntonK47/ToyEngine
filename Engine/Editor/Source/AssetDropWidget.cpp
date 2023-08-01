#include "AssetDropWidget.h"

#include <ImGuiNodeEditor.h>
#include <format>

using namespace toy::editor;
using namespace toy::editor::widget;

auto toy::editor::widget::assetDropTarget(const char* label, const std::string_view AssetObjectType, AssetEntry* entry, const AssetDatabase& db) -> bool
{
    TOY_ASSERT(entry);
    auto dropped = false;
    auto name = entry? entry->name : std::string{};
 
    ImGui::InputTextWithHint(std::format("{} {}", ICON_FA_ARROW_UP_RIGHT_FROM_SQUARE, label).c_str(), std::format("{}", AssetObjectType).c_str(), &name, ImGuiInputTextFlags_ReadOnly);
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::format("AssetPayload{}", AssetObjectType).c_str()))
        {
            int index = *(const int*)payload->Data;
            if (!db.db.entries.empty())
            {
                *entry = db.db.entries[index];
                dropped = true;
            }
        }
        ImGui::EndDragDropTarget();
    }
    return dropped;
}