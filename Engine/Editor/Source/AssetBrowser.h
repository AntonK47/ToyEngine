#pragma once

#include <Editor.h>
#include <ImGuiNodeEditor.h>
#include <vector>

namespace toy::editor
{
	enum class BrowserView
	{
		list,
		grid
	};

	struct AssetDatabase;

	struct AssetBrowserDescriptor
	{
		AssetDatabase* database;
	};

	class AssetBrowser final : public Editor
	{
	public:
		auto initialize(const AssetBrowserDescriptor& descriptor) -> void;
		auto deinitialize() -> void;
	private:
		auto onDrawGui() -> void override;
		ImGuiTextFilter filter_;
		BrowserView view_{ BrowserView::list };
		std::vector<int> selectedAssets_{};
		AssetDatabase* database_{ nullptr };
	};
}