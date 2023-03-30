#include "VulkanRHI/VulkanMappings.h"

#include "VulkanRHI/Vulkan.h"
#include <fstream>

using namespace toy::core;

namespace toy::graphics::rhi::vulkan
{
	auto vulkanMapImageAccessUsageFlag(const Flags<ImageAccessUsage>& usage) -> vk::ImageUsageFlags
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

	auto vulkanMapBufferAccessUsageFlag(
		const Flags<BufferAccessUsage>& usage) -> vk::BufferUsageFlags
	{
		auto vulkanUsage = vk::BufferUsageFlags{};
		//TODO: Add new ray tracing specific access usages
		if (usage.containBit(BufferAccessUsage::accelerationStructure))
		{
			vulkanUsage |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
		}
		if (usage.containBit(BufferAccessUsage::vertex))
		{
			vulkanUsage |= vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
		}
		if (usage.containBit(BufferAccessUsage::index))
		{
			vulkanUsage |= vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
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

	auto mapExtent(const Extent& extent) -> vk::Extent3D
	{
		return vk::Extent3D{ extent.width, extent.height, extent.depth };
	}

	auto mapViewType(ImageViewType type) -> vk::ImageViewType
	{
		return vk::ImageViewType::e2D;//TODO: replace this placeholder
	}

	auto loadShader(const vk::Device device,
		const std::string& path) -> vk::ShaderModule
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

	auto mapFormat(const Format format) -> vk::Format
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
		case Format::r8:
			vulkanFormat = vk::Format::eR8Unorm;
		}
		return vulkanFormat;
	}

	auto mapColorFormat(const ColorFormat format) -> vk::Format
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

	auto mapDepthFormat(const DepthFormat format) -> vk::Format
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

	auto mapDescriptorType(const BindingType type) -> vk::DescriptorType
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

	auto mapViewAspect(const ImageViewAspect aspect) -> vk::ImageAspectFlagBits
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

	auto mapFilter(Filter filter) -> vk::Filter
	{
		switch (filter)
		{
		case Filter::nearest:
			return vk::Filter::eNearest;
		case Filter::linear:
			return vk::Filter::eLinear;
		case Filter::cubic:
			return vk::Filter::eCubicIMG;
		}
		return vk::Filter::eNearest;
	}

	auto mapMipFilter(MipFilter filter) -> vk::SamplerMipmapMode
	{
		switch (filter)
		{
		case MipFilter::nearest:
			return vk::SamplerMipmapMode::eNearest;
		case MipFilter::linear:
			return vk::SamplerMipmapMode::eLinear;
		}
		return vk::SamplerMipmapMode::eNearest;
	}
}