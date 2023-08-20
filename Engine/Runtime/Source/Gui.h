#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <IconsFontAwesome6.h>
#include <IconsFontAwesome6Brands.h>
#include <imgui_node_editor.h>

#include <RenderInterface.h>
#include "Statistics.h"
#include "TextureManager.h"
#include <DynamicFrameAllocator.h>

namespace toy
{
	struct GuiDescriptor
	{
		graphics::rhi::RenderInterface& renderer;
		TextureManager& textureManager;
		ImageDataUploader& imageUploader;
		toy::graphics::DynamicFrameAllocator& dynamicFrameAllocator;
		float dpiScale;
	};

	class Gui final
	{
	public:
		auto initialize(const GuiDescriptor& descriptor) -> void;
		auto deinitialize() -> void;

		[[nodiscard]] inline auto getIo() -> ImGuiIO&
		{
			return ImGui::GetIO();
		}

		auto render() -> void
		{
			ImGui::Render();
		}
		auto fillCommandList(toy::graphics::rhi::CommandList& cmd) -> GuiDrawStatistics;

	private:
		graphics::rhi::Handle<graphics::rhi::Pipeline> guiPipeline_;
		graphics::rhi::Handle<graphics::rhi::BindGroup> guiFontBindGroup_;
		graphics::rhi::Handle<graphics::rhi::BindGroupLayout> guiVertexDataGroupLayout_;

		graphics::rhi::RenderInterface* renderer_;
		TextureManager* textureManager_;
		toy::graphics::DynamicFrameAllocator* dynamicFrameAllocator_;
	};
}