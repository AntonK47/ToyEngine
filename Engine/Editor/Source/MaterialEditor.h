#pragma once
#include "Editor.h"
#include "Core.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <array>
#include <memory>
#include <format>
#include <concepts>
#include <Logger.h>

#include <MaterialEditorResolver.h>
#include <ImGuiNodeEditor.h>
#include <MaterialModel.h>
#include <Undo.h>

namespace toy::editor
{
	struct InputPin;
	struct OutputPin;

	class MaterialEditor final : public Editor, Document, MaterialModel, public Undo
	{
	public:
		auto initialize() -> void;
		auto deinitialize() -> void;

		inline auto nodes() -> std::vector<std::unique_ptr<MaterialNode>>& override
		{
			return nodes_;
		}
		
		inline auto links() -> std::vector<std::unique_ptr<Link>>& override
		{
			return links_;
		}
	private:

		auto drawNode(MaterialNode& node) -> void;
		auto drawMaterialEditor() -> void;
		void onDrawGui() override;
		bool hasAnyCircle(MaterialNode* root);

		ed::NodeId selectedNodeId{};
		ed::PinId selectedPinId{};
		ed::LinkId selectedLinkId{};

		ImVec2 openPopupPosition_{};

		ed::EditorContext* context_;
		bool isFirstFrame_ = true;
		std::vector<std::unique_ptr<MaterialNode>> nodes_;
		std::vector<std::unique_ptr<Link>> links_;

		std::unique_ptr<resolver::MaterialEditorResolver> resolver_;
	};
}

