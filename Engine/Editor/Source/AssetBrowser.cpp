#include "AssetBrowser.h"

#include <ImGuiNodeEditor.h>
#include <format>
#include <AssetDatabase.h>

using namespace toy::editor;

auto AssetBrowser::initialize(const AssetBrowserDescriptor& descriptor) -> void
{
    database_ = descriptor.database;
}

auto AssetBrowser::deinitialize() -> void
{
}

auto AssetBrowser::onDrawGui() -> void
{
    filter_.Draw(ICON_FA_FILTER);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
    {
        ImGui::SetItemTooltip(std::format("{} {}", ICON_FA_SCREWDRIVER_WRENCH, "IN DEVELOPMENT!").c_str());
    }
    ImGui::SameLine(0.0f, 40.0f);

    auto isGridView = view_ == BrowserView::grid;
    auto isListView = view_ == BrowserView::list;
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));
    ImGui::ToggleButton(std::format("{}##gridViewToggle", ICON_FA_GRIP).c_str(), &isGridView);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
    {
        ImGui::SetItemTooltip(std::format("{} {}", ICON_FA_SCREWDRIVER_WRENCH, "IN DEVELOPMENT!").c_str());
    }
    ImGui::SameLine();
    ImGui::ToggleButton(std::format("{}##gridViewToggle", ICON_FA_LIST).c_str(), &isListView);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
    {
        ImGui::SetItemTooltip(std::format("{} {}", ICON_FA_SCREWDRIVER_WRENCH, "IN DEVELOPMENT!").c_str());
    }
    ImGui::PopStyleVar();

    if (isListView && isGridView)
    {
        if (view_ == BrowserView::grid)
        {
            view_ = BrowserView::list;
        }
        else
        {
            view_ = BrowserView::grid;
        }
    }

    const auto textHeight = ImGui::GetTextLineHeightWithSpacing();
    auto flags = ImGuiTableFlags_ScrollY;
    ImVec2 outer_size = ImVec2(0.0f, textHeight * 8);
    if (ImGui::BeginTable("assetBrowser", 4, flags, outer_size))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_None);
        ImGui::TableHeadersRow();

        const auto selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;

        ImGuiListClipper clipper;
        clipper.Begin(database_->db.entries.size());
        while (clipper.Step())
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; index++)
            {
                auto selected = std::find(selectedAssets_.begin(), selectedAssets_.end(), index) != selectedAssets_.end();

                const auto& entry = database_->db.entries[index];

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(std::format("{}##{}", ICON_FA_FILE, index).c_str(), selected, selectableFlags, ImVec2(0, 0)))
                {
                   /* if (ImGui::GetIO().KeyCtrl)
                    {
                        if (selected)
                            selection.find_erase_unsorted(item->ID);
                        else
                            selected.push_back(item->ID);
                    }
                    else
                    {
                        selection.clear();
                        selection.push_back(item->ID);
                    }*/
                    selectedAssets_.clear();
                    selectedAssets_.push_back(index);
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload(std::format("AssetPayload{}", entry.type).c_str(), &index, sizeof(int));
                    ImGui::Text(entry.name.c_str());
                    ImGui::EndDragDropSource();
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(entry.name.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text(entry.type.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text(entry.coreFilePath.c_str());
            }
        }
        ImGui::EndTable();
    }
}
