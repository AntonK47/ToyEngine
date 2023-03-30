#pragma once
#include "VulkanRHI/Vulkan.h"
#include "RenderInterfaceTypes.h"

namespace toy::graphics::rhi::vulkan
{
    using namespace toy::core;

	vk::BufferUsageFlags vulkanMapBufferAccessUsageFlag(const Flags<BufferAccessUsage>& usage);

    vk::ImageUsageFlags vulkanMapImageAccessUsageFlag(const Flags<ImageAccessUsage>& usage);

    vk::Extent3D mapExtent(const Extent& extent);

    vk::ImageViewType mapViewType(ImageViewType type);

    vk::ShaderModule loadShader(vk::Device device, const std::string& path);
    //TODO:: Formats should be queries from the formats table, witch is created after device selection
    vk::Format mapFormat(Format format);

    vk::Format mapColorFormat(ColorFormat format);

    vk::Format mapDepthFormat(DepthFormat format);

    vk::DescriptorType mapDescriptorType(BindingType type);

    vk::ImageAspectFlagBits mapViewAspect(ImageViewAspect aspect);

    [[nodiscard]] auto mapFilter(Filter filter) -> vk::Filter;
    [[nodiscard]] auto mapMipFilter(MipFilter filter) -> vk::SamplerMipmapMode;

	[[nodiscard]] auto mapResolve(ResolveMode resolveMode) -> vk::ResolveModeFlagBits
	{
		return vk::ResolveModeFlagBits::eNone;
	}

	[[nodiscard]] auto mapLoadOp(LoadOperation load) -> vk::AttachmentLoadOp
	{
		switch (load)
		{
		case LoadOperation::load:
			return vk::AttachmentLoadOp::eLoad;
			break;
		case LoadOperation::clear:
			return vk::AttachmentLoadOp::eClear;
			break;
		case LoadOperation::dontCare:
			return vk::AttachmentLoadOp::eDontCare;
			break;
		case LoadOperation::none:
			return vk::AttachmentLoadOp::eNoneEXT;
			break;
		default:
			break;
		}
		return vk::AttachmentLoadOp::eClear;

	}

	[[nodiscard]] auto mapClearValue(const ColorClear& colorClear) -> vk::ClearValue
	{
		return vk::ClearValue{ vk::ClearColorValue{ std::array<float,4>{colorClear.r,colorClear.g,colorClear.b,colorClear.a} } };
	}

	 [[nodiscard]] auto mapDepthClearValue(const DepthClear& colorClear) -> vk::ClearValue
	{
		return vk::ClearValue
		{
			.depthStencil = vk::ClearDepthStencilValue
			{
				colorClear.depth,
				u32{0}
			}
		};
	}

	[[nodiscard]] auto mapStoreOp(StoreOperation store) -> vk::AttachmentStoreOp
	{
		switch (store)
		{
		case StoreOperation::store:
			return vk::AttachmentStoreOp::eStore;
			break;
		case StoreOperation::dontCare:
			return vk::AttachmentStoreOp::eDontCare;
			break;
		case StoreOperation::none:
			return vk::AttachmentStoreOp::eNoneKHR;
			break;
		default:
			break;
		}
		return vk::AttachmentStoreOp::eStore;
	}
}
