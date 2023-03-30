#pragma once
#include <utility>
#include "RenderInterfaceTypes.h"
#include "RenderInterface.h"

#include <Core.h>
#include <variant>
#include <g3log/g3log.hpp>

#include <type_traits>

namespace toy::graphics::rhi
{
	struct VirtualTextureStreamingDescriptor
	{
		//RenderInterface& renderInterface;
	};

	struct VirtualTextureStreamingSamplingFeedback
	{};

	template<core::u8 PageSizeX, core::u8 PageSizeY>
	struct StreamingPageMemoryPool //add strategy for full pool behavier
	{

		core::u32 pageSize;

	};


	struct Point
	{
		core::u32 x;
		core::u32 y;
	};

	struct Extent2D
	{
		core::u32 width;
		core::u32 height;
	};

	struct Rectangle
	{
		Point offset;
		Extent2D size;
	};

	struct TextureRegion
	{
		core::u8 mipLevel;
		core::u8 layer;
		Rectangle region;
	};

	template <typename T>
	concept Texture2DLoader = requires(T a, const auto& b, const TextureRegion& c)
	{
		{a.load(b, c)};
	};
	
	struct DDSTextureDataDescriptor
	{
	};

	template <Texture2DLoader Loader>
	class Texture2DAsyncDataSource
	{
		

	public:

		auto request(const auto& textureDataDescriptor, const TextureRegion& region)
		{
			loader_.load(textureDataDescriptor, region);
		}
	private:

		Loader loader_;
	};

	struct PageExtracter
	{
		
	};

	class DDSTextureLoader
	{
	public:
		using TextureDataDescriptorType = DDSTextureDataDescriptor;

		auto load(const DDSTextureDataDescriptor& descriptor, const TextureRegion& region) -> void
		{
			LOG(INFO) << "DDS Texture loading...!";
		}
	};

	class VirtualTextureStreaming
	{
	public:
		auto initialize(const VirtualTextureStreamingDescriptor& descriptor) -> void
		{
			//renderInterface_ = *descriptor.renderInterface;
			auto dataSource = Texture2DAsyncDataSource<DDSTextureLoader>{};

			auto testDescriptor = DDSTextureDataDescriptor{};

			dataSource.request(testDescriptor, TextureRegion{});
		}

		auto deinitialize() -> void
		{
			
		}

		auto createVirtualTexture() -> VirtualTexture
		{

		}

		auto registerVirtualTexture(const VirtualTexture& virtualTexture) -> void
		{
		}

	private:

		//RenderInterface* renderInterface_;
		StreamingPageMemoryPool<128, 128> memoryPool_;

	
	private:
		//memmory pool
		//tile loading queue
		//
	};
}