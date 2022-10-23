#pragma once
#include "Vulkan.h"
#include "RenderInterfaceCommonTypes.h"

namespace toy::renderer
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
}
