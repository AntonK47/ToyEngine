#include "VulkanMappings.h"

#include "Vulkan.h"
#include "Structs.h"

#include <fstream>

using namespace toy::core;
using namespace toy::renderer;

vk::ImageUsageFlags toy::renderer::vulkanMapImageAccessUsageFlag(const Flags<ImageAccessUsage>& usage)
{
    auto vulkanUsage = vk::ImageUsageFlags{};

    if (usage.containBit(ImageAccessUsage::colorAttachment))
    {
        vulkanUsage |= vk::ImageUsageFlagBits::eColorAttachment;
    }
    if (usage.containBit(ImageAccessUsage::depthStencilAttachment))
    {
        vulkanUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }
    if (usage.containBit(ImageAccessUsage::sampled))
    {
        vulkanUsage |= vk::ImageUsageFlagBits::eSampled;
    }
    if (usage.containBit(ImageAccessUsage::storage))
    {
        vulkanUsage |= vk::ImageUsageFlagBits::eStorage;
    }
    if (usage.containBit(ImageAccessUsage::transferDst))
    {
        vulkanUsage |= vk::ImageUsageFlagBits::eTransferDst;
    }
    if (usage.containBit(ImageAccessUsage::transferSrc))
    {
        vulkanUsage |= vk::ImageUsageFlagBits::eTransferSrc;
    }


    return vulkanUsage;
}

vk::BufferUsageFlags toy::renderer::vulkanMapBufferAccessUsageFlag(
	const Flags<BufferAccessUsage>& usage)
{
	auto vulkanUsage = vk::BufferUsageFlags{};

	if (usage.containBit(BufferAccessUsage::accelerationStructure))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
	}
	if (usage.containBit(BufferAccessUsage::vertex))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
	}
	if (usage.containBit(BufferAccessUsage::index))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
	}
	if (usage.containBit(BufferAccessUsage::indirect))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eIndirectBuffer;
	}
	if (usage.containBit(BufferAccessUsage::uniform))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
	}
	if (usage.containBit(BufferAccessUsage::storage))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
	}
	if (usage.containBit(BufferAccessUsage::transferDst))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eTransferDst;
	}
	if (usage.containBit(BufferAccessUsage::transferSrc))
	{
		vulkanUsage |= vk::BufferUsageFlagBits::eTransferSrc;
	}

	return vulkanUsage;
}

vk::Extent3D toy::renderer::mapExtent(const Extent& extent)
{
	return vk::Extent3D{ extent.width, extent.height, extent.depth };
}

vk::ImageViewType toy::renderer::mapViewType(ImageViewType type)
{
	return vk::ImageViewType::e2D;//TODO: replace this placeholder
}

vk::ShaderModule toy::renderer::loadShader(const vk::Device device,
	const std::string& path)
{
	auto length = uint32_t{};
	auto buffer = static_cast<char*>(nullptr);
	{
		using namespace std::string_literals;
		const auto& filePath = path;
		auto fileStream = std::ifstream{ filePath, std::ios::binary | std::ios::ate };
		TOY_ASSERT(fileStream.is_open());
		length = static_cast<uint32_t>(fileStream.tellg());
		fileStream.seekg(0);

		buffer = new char[length];
		fileStream.read(buffer, length);
		fileStream.close();
	}

	const auto moduleCreateInfo = vk::ShaderModuleCreateInfo
	{
		.codeSize = static_cast<size_t>(length),
		.pCode = reinterpret_cast<const uint32_t*>(buffer)
	};

	return device.createShaderModule(moduleCreateInfo).value;
}

vk::Format toy::renderer::mapFormat(const Format format)
{
	auto vulkanFormat = vk::Format::eB8G8R8A8Unorm;
	switch (format)
	{
	case Format::rgba8:
		vulkanFormat = vk::Format::eB8G8R8A8Unorm;
		break;
	case Format::rgba16:
		vulkanFormat = vk::Format::eR16G16B16A16Snorm;
		break;
	case Format::d16:
		vulkanFormat = vk::Format::eD16Unorm;
		break;
	case Format::d32:
		vulkanFormat = vk::Format::eD32Sfloat;
		break;
	}
	return vulkanFormat;
}

vk::Format toy::renderer::mapColorFormat(const ColorFormat format)
{
	auto vulkanFormat = vk::Format::eB8G8R8A8Unorm;
	switch (format)
	{
	case ColorFormat::rgba8:
		vulkanFormat = vk::Format::eB8G8R8A8Unorm;
		break;
	case ColorFormat::rgba16:
		vulkanFormat = vk::Format::eR16G16B16A16Snorm;
		break;
	}
	return vulkanFormat;
}

vk::Format toy::renderer::mapDepthFormat(const DepthFormat format)
{
	auto vulkanFormat = vk::Format::eB8G8R8A8Unorm;
	switch (format)
	{
	case DepthFormat::d16:
		vulkanFormat = vk::Format::eD16Unorm;
		break;
	case DepthFormat::d32:
		vulkanFormat = vk::Format::eD32Sfloat;
		break;
	}
	return vulkanFormat;
}

vk::DescriptorType toy::renderer::mapDescriptorType(const BindingType type)
{
	switch (type)
	{
	case BindingType::Texture1D:

	case BindingType::Texture2D:

	case BindingType::Texture3D:

	case BindingType::Texture2DArray:
		return vk::DescriptorType::eSampledImage;
	case BindingType::UniformBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case BindingType::StorageBuffer:
		return vk::DescriptorType::eStorageBuffer;

	case BindingType::AccelerationStructure:
		return vk::DescriptorType::eAccelerationStructureKHR;

	case BindingType::Sampler:
		return vk::DescriptorType::eSampler;
	}

	return vk::DescriptorType::eStorageBuffer;
}

vk::ImageAspectFlagBits toy::renderer::mapViewAspect(const ImageViewAspect aspect)
{

	switch (aspect)
	{
	case ImageViewAspect::color:
		return vk::ImageAspectFlagBits::eColor;
	case ImageViewAspect::depth:
		return  vk::ImageAspectFlagBits::eDepth;
	}

	return vk::ImageAspectFlagBits::eNone;
}
