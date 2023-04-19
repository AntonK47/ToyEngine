#pragma once

#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>
#include <Core.h>
#include <unordered_map>
#include <vector>
#include "Texture2D.h"

namespace toy
{
	struct TextureManagerDescriptor
	{
		RenderInterface& rhi;
		core::u32 frameInFlights;
	};

	class TextureManager
	{
	public:
		auto initialize(const TextureManagerDescriptor& descriptor) -> void;
		auto nextFrame() -> void;
		auto addTexture(const Texture2D& texture) -> core::UID;
		auto get(const core::UID uid) -> Texture2D&;
		auto getBindIndex(const core::UID uid) -> core::u32;
		auto updateBindGroup() -> void;
		auto getTextureBindGroup() -> toy::graphics::rhi::Handle<toy::graphics::rhi::BindGroup>;
		auto getTextureBindGroupLayout() -> toy::graphics::rhi::Handle<toy::graphics::rhi::BindGroupLayout>;

	private:
		std::vector<Texture2D> textures_{};
		std::unordered_map<core::UID, core::u32> uidToIndexMap_{};
		std::unordered_map<core::UID, core::u32> uidToBindingMap_{};

		std::vector<std::vector<core::UID>> deletionQueuePerFrame_;

		bool needUpdate_{ false };

		std::vector<toy::graphics::rhi::Handle<toy::graphics::rhi::BindGroup>> texture2DGroupPerFrame_;
		toy::graphics::rhi::Handle<toy::graphics::rhi::BindGroupLayout> texture2DGroupLayout_;

		core::u32 currentFrameIndex_{};
		core::u32 framesInFlight_{};

		const core::u32 maxDescriptorsPerGroup_{ 20 };

		toy::graphics::rhi::RenderInterface* rhi_;
	};
}