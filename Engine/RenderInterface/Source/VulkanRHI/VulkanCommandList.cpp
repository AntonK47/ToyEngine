#include "VulkanRHI/VulkanCommandList.h"
#include "VulkanRHI/VulkanMappings.h"
#include "VulkanRHI/VulkanRenderInterface.h"

#include "RenderInterfaceTypes.h"
#include "VulkanRHI/VulkanRenderInterfaceTypes.h"

#include "VulkanRHI/Vulkan.h"

using namespace toy::graphics::rhi;
using namespace vulkan;

namespace
{
	

	bool hasDepthAttachment(const RenderingDescriptor& descriptor)
	{
		return descriptor.depthRenderTarget.has_value();
	}

	bool hasStencilAttachment(const RenderingDescriptor& descriptor)
	{
		return descriptor.stencilRenderTarget.has_value();
	}

	vk::GeometryFlagsKHR mapGeometryFlags(GeometryBehavior behavier)
	{
		return vk::GeometryFlagsKHR{};
	}
}

auto VulkanSubmitBatch::barrierInternal() -> SubmitDependency
{
	auto value = u64{};
	switch(queueType_)
	{
	case QueueType::graphics: 
		value = batch_.waitGraphicsValue;
		break;
	case QueueType::asyncCompute:
		value = batch_.waitAsyncComputeValue;
		break;
	case QueueType::transfer:
		value = batch_.waitTransferValue;
		break;
	default: ;
	}

	return SubmitDependency
	{
		queueType_,
		value + 1
	};
}


void VulkanCommandList::barrierInternal(const std::initializer_list<BarrierDescriptor>& descriptors)
{
	auto imageBarriers = std::vector<vk::ImageMemoryBarrier2>{};

	for(const auto& barrierDescriptor: descriptors)
	{
		if(std::holds_alternative<ImageBarrierDescriptor>(barrierDescriptor))
		{
			const auto& imageBarrierDescriptor = std::get<ImageBarrierDescriptor>(barrierDescriptor);

			auto barrier = vk::ImageMemoryBarrier2{};
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = rhi_.imageStorage_.get(imageBarrierDescriptor.image).image;
			barrier.subresourceRange = vk::ImageSubresourceRange
			{
				mapViewAspect(imageBarrierDescriptor.aspect), 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS
			};
			//TODO: it might be incorrect!!
			switch (imageBarrierDescriptor.srcLayout)
			{
			case Layout::undefined:
				barrier.oldLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
				barrier.dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
				break;
			case Layout::colorRenderTarget:
				barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
				barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
				break;
			case Layout::present:
				barrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			case Layout::transferDst:
				barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
				barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
				break;
			case Layout::transferSrc:
				barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
				barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
				barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
				break;
			default:

				barrier.newLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;

				break;
			}

			switch (imageBarrierDescriptor.dstLayout)
			{
			case Layout::colorRenderTarget:
				barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
				barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
				break;
			case Layout::depthStencilRenderTarget:
				barrier.newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eLateFragmentTests;
				barrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
				break;
			case Layout::present:
				barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			case Layout::transferDst:
				barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
				barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
				break;
			case Layout::transferSrc:
				barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
				barrier.dstAccessMask = vk::AccessFlagBits2::eTransferRead;
				break;
			case Layout::shaderRead: //TODO: this enum name is not clear about further resource usage
				barrier.newLayout = vk::ImageLayout::eReadOnlyOptimal;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
				barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
				break;
			default:

				barrier.newLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			}
			
			imageBarriers.push_back(barrier);  //TODO: << memory allocation
		}
	}

	const auto dependency = vk::DependencyInfo
	{
		.dependencyFlags = vk::DependencyFlagBits::eByRegion,
		.memoryBarrierCount = 0,
		.pMemoryBarriers = nullptr,
		.bufferMemoryBarrierCount = 0,
		.pBufferMemoryBarriers = nullptr,
		.imageMemoryBarrierCount = static_cast<u32>(imageBarriers.size()),
		.pImageMemoryBarriers = imageBarriers.data()
	};

	commandBuffer_.pipelineBarrier2(dependency);  //TODO: vulan driver waits???

}

void VulkanCommandList::beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area)
{
	const auto containsSecondaryBuffers = false;
	auto flags = vk::RenderingFlags{};
	if(containsSecondaryBuffers)
	{
		flags |= vk::RenderingFlagBits::eContentsSecondaryCommandBuffers;
	}
	

	auto colorAttachments = std::vector<vk::RenderingAttachmentInfo>(descriptor.colorRenderTargets.size());//TODO: smallvector

	for (u32 i{}; i< descriptor.colorRenderTargets.size(); i++)
	{
		const auto& colorRenderTarget = descriptor.colorRenderTargets[i];

		const auto& imageView = rhi_.imageViewStorage_.get(colorRenderTarget.imageView).imageView;

		colorAttachments[i] = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.resolveMode = mapResolve(colorRenderTarget.resolveMode),
			.loadOp = mapLoadOp(colorRenderTarget.load),
			.storeOp = mapStoreOp(colorRenderTarget.store),
			.clearValue = mapClearValue(std::get<ColorClear>(colorRenderTarget.clearValue))
		};
	}

	auto depthAttachment = vk::RenderingAttachmentInfo{};

	if(hasDepthAttachment(descriptor))
	{
		const auto& renderTarget = descriptor.depthRenderTarget.value();
		const auto& imageView = rhi_.imageViewStorage_.get(renderTarget.imageView).imageView;
		depthAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.resolveMode = mapResolve(renderTarget.resolveMode),
			.loadOp = mapLoadOp(renderTarget.load),
			.storeOp = mapStoreOp(renderTarget.store),
			.clearValue = mapDepthClearValue(std::get<DepthClear>(renderTarget.clearValue))
		};
	}

	auto stencilAttachment = vk::RenderingAttachmentInfo{};
	if(hasStencilAttachment(descriptor))
	{
		const auto& renderTarget = descriptor.stencilRenderTarget.value();
		const auto& imageView = rhi_.imageViewStorage_.get(renderTarget.imageView).imageView;
		stencilAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eStencilAttachmentOptimal,
			.resolveMode = mapResolve(renderTarget.resolveMode),
			.loadOp = mapLoadOp(renderTarget.load),
			.storeOp = mapStoreOp(renderTarget.store),
			//.clearValue = mapClearValue(descriptor.stencilRenderTarget.clearValue)TODO:??
		};
	}

	const auto renderingInfo = vk::RenderingInfo
	{
		.flags = flags,
		.renderArea = vk::Rect2D{ vk::Offset2D{ area.x, area.y }, vk::Extent2D{area.width, area.height} },
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = static_cast<u32>(colorAttachments.size()),
		.pColorAttachments = colorAttachments.data(),
		.pDepthAttachment = hasDepthAttachment(descriptor)?&depthAttachment:nullptr,
		.pStencilAttachment = hasStencilAttachment(descriptor)?&stencilAttachment:nullptr
	};

	commandBuffer_.beginRendering(renderingInfo);
}

void VulkanCommandList::endRenderingInternal()
{
	commandBuffer_.endRendering();
}

void VulkanCommandList::drawInternal(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
{
	
	commandBuffer_.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::drawIndexedInternal(
	core::u32 indexCount,
	core::u32 instanceCount,
	core::u32 firstIndex,
	core::i32 vertexOffset,
	core::u32 firstInstance
)
{
	commandBuffer_.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstIndex);
}

void VulkanCommandList::transferInternal(const TransferBufferDescriptor& srcBufferDescriptor, const TransferImageDescriptor& dstImageDescription)
{
	TOY_ASSERT(!dstImageDescription.regions.empty());

	auto regions = std::vector<vk::BufferImageCopy2>{};
	regions.resize(dstImageDescription.regions.size());

	const auto& buffer = rhi_.bufferStorage_.get(srcBufferDescriptor.buffer);
	const auto& image = rhi_.imageStorage_.get(dstImageDescription.image);

	for (u32 i = {}; i < dstImageDescription.regions.size(); i++)
	{
		const auto& region = dstImageDescription.regions[i];
		const auto subresource = vk::ImageSubresourceLayers
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,// image.aspect, TODO:
			.mipLevel = region.mip,
			.baseArrayLayer = region.baseLayer,
			.layerCount = region.layerCount
		};
		regions[i] = vk::BufferImageCopy2
		{
			.bufferOffset = srcBufferDescriptor.offset + region.bufferOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = subresource,
			.imageOffset = vk::Offset3D{region.offset.x, region.offset.y, region.offset.z},
			.imageExtent = vk::Extent3D{ region.extent.x, region.extent.y, region.extent.z }
		};
	}

	const auto copyBufferToImageInfo = vk::CopyBufferToImageInfo2
	{
		.srcBuffer = buffer.buffer,
		.dstImage = image.image,
		.dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
		.regionCount = static_cast<u32>(regions.size()),
		.pRegions = regions.data()
	};

	commandBuffer_.copyBufferToImage2(copyBufferToImageInfo);
}

void VulkanCommandList::transferInternal(
	const TransferImageDescriptor& srcImageDescriptor,
	const TransferBufferDescriptor& dstBufferDescription)
{
	TOY_ASSERT(!srcImageDescriptor.regions.empty());

	auto regions = std::vector<vk::BufferImageCopy2>{};
	regions.resize(srcImageDescriptor.regions.size());

	const auto& buffer = rhi_.bufferStorage_.get(dstBufferDescription.buffer);
	const auto& image = rhi_.imageStorage_.get(srcImageDescriptor.image);

	for (u32 i = {}; i < srcImageDescriptor.regions.size(); i++)
	{
		const auto& region = srcImageDescriptor.regions[i];
		const auto subresource = vk::ImageSubresourceLayers
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,// image.aspect, TODO:
			.mipLevel = region.mip,
			.baseArrayLayer = region.baseLayer,
			.layerCount = region.layerCount
		};
		regions[i] = vk::BufferImageCopy2
		{
			.bufferOffset = dstBufferDescription.offset + region.bufferOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = subresource,
			.imageOffset = vk::Offset3D{region.offset.x, region.offset.y, region.offset.z},
			.imageExtent = vk::Extent3D{ region.extent.x, region.extent.y, region.extent.z }
		};
	}

	const auto copyImageToBufferInfo = vk::CopyImageToBufferInfo2
	{
		.srcImage = image.image,
		.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
		.dstBuffer = buffer.buffer,
		.regionCount = static_cast<u32>(regions.size()),
		.pRegions = regions.data()
	};

	commandBuffer_.copyImageToBuffer2(copyImageToBufferInfo);
}

void VulkanCommandList::bindPipelineInternal(const Handle<Pipeline>& pipeline)
{
	currentPipeline_ = rhi_.pipelineStorage_.get(pipeline);
	commandBuffer_.bindPipeline(currentPipeline_.bindPoint, currentPipeline_.pipeline);
}

void VulkanCommandList::bindIndexBufferInternal(const Handle<Buffer>& buffer, const u64 offset, const IndexType indexType)
{
	const auto& indexBuffer = rhi_.bufferStorage_.get(buffer);

	const auto type = indexType == IndexType::index16 ? vk::IndexType::eUint16 : vk::IndexType::eUint32; //TODO: maybe i should consider also 8 bit index types
	commandBuffer_.bindIndexBuffer(indexBuffer.buffer, vk::DeviceSize{ offset }, type);
}

void VulkanCommandList::setScissorInternal(const Scissor& scissor)
{
	const auto vulkanScissor = vk::Rect2D{ {scissor.x,scissor.y},{scissor.width, scissor.height}};
	
	commandBuffer_.setScissor(0, 1, &vulkanScissor);
}

void VulkanCommandList::setViewportInternal(
	const Viewport& viewport)
{
	//TODO: flip viewport???
	const auto vulkanViewport = vk::Viewport{ viewport.x,viewport.y + viewport.height,viewport.width,-viewport.height,0.0,1.0 };

	//TODO:: Whats about depth value?

	commandBuffer_.setViewport(0, 1, &vulkanViewport);
}

void VulkanCommandList::bindGroupInternal(
	const u32 set,
	const Handle<BindGroup>& handle)
{
	auto vulkanBindGroup = VulkanBindGroup{};
	//TODO: do something clever
	if (rhi_.persistentBindGroupStorage_.contains(handle))
	{
		vulkanBindGroup = rhi_.persistentBindGroupStorage_.get(handle);
	}
	else
	{
		vulkanBindGroup = rhi_.bindGroupStorage_.get(handle);
	}
	//const auto& vulkanBindGroup = renderInterface_->bindGroupStorage_.get(handle);
	


	commandBuffer_.bindDescriptorSets(currentPipeline_.bindPoint, currentPipeline_.layout, set, 1, &vulkanBindGroup.descriptorSet, 0, nullptr);
}

std::vector<Handle<AccelerationStructure>> VulkanCommandList::
buildAccelerationStructureInternal(const TriangleGeometry& geometry,
	const std::vector<AccelerationStructureDescriptor>& descriptors)
{
	const auto vertexData = rhi_.device_.getBufferAddress(
		vk::BufferDeviceAddressInfo
		{
			.buffer = rhi_.bufferStorage_.get(geometry.vertexBuffer).buffer
		});

	const auto indexData = rhi_.device_.getBufferAddress(
		vk::BufferDeviceAddressInfo
		{
			.buffer = rhi_.bufferStorage_.get(geometry.indexBuffer).buffer
		});

	const auto accelerationStructureGeometry = vk::AccelerationStructureGeometryKHR
	{
		.geometryType = vk::GeometryTypeKHR::eTriangles,
		.geometry =
		{
			.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR
			{
				.vertexFormat = vk::Format::eR32G32B32Sfloat,
				.vertexData = {vertexData},
				.vertexStride = geometry.vertexStride,
				.maxVertex = geometry.totalVertices,
				.indexType = vk::IndexType::eUint32, //TODO pick the right one
				.indexData = {indexData},
				.transformData = {0}
			}
		},
		.flags = mapGeometryFlags(geometry.behavior)
	};

	auto properties = vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceAccelerationStructurePropertiesKHR>{};
	rhi_.adapter_.getProperties2(&properties.get());

	const auto accelerationStructureProperties = properties.get<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();

	auto buildInfos = std::vector<vk::AccelerationStructureBuildGeometryInfoKHR>{};
	buildInfos.resize(descriptors.size());

	auto buildRages = std::vector<vk::AccelerationStructureBuildRangeInfoKHR>{};
	buildInfos.resize(descriptors.size());

	auto buildSizes = std::vector<vk::AccelerationStructureBuildSizesInfoKHR>{};
	buildSizes.resize(descriptors.size());

	auto totalAccelerationStructureBufferSize = u32{};
	auto totalScratchBufferSize = u32{};

	auto accelerationStructureBufferOffsets = std::vector<u32>{};
	accelerationStructureBufferOffsets.resize(descriptors.size());

	auto scratchBufferOffsets = std::vector<u32>{};
	scratchBufferOffsets.resize(descriptors.size());

	for (auto i = u32{}; i < descriptors.size(); i++)
	{
		buildInfos[i] = vk::AccelerationStructureBuildGeometryInfoKHR
		{
			.type = vk::AccelerationStructureTypeKHR::eBottomLevel,
			.mode = vk::BuildAccelerationStructureModeKHR::eBuild,
			.srcAccelerationStructure = VK_NULL_HANDLE,
			.geometryCount = 1,
			.pGeometries = &accelerationStructureGeometry
		};

		const auto maxPrimitiveCount = descriptors[i].primitiveCount; //TODO:!!!!!!!!!!!

		TOY_ASSERT_BREAK(maxPrimitiveCount <= accelerationStructureProperties.maxInstanceCount);

		buildSizes[i] = rhi_.device_.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfos[i], maxPrimitiveCount);


		scratchBufferOffsets[i] = totalScratchBufferSize;
		accelerationStructureBufferOffsets[i] = totalAccelerationStructureBufferSize;

		totalAccelerationStructureBufferSize += buildSizes[i].accelerationStructureSize + 256 - buildSizes[i].accelerationStructureSize % 256;
		totalScratchBufferSize += buildSizes[i].buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment - buildSizes[i].buildScratchSize % accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
	}

	const auto accelerationStructureBuffer = rhi_.createBuffer(
		BufferDescriptor{
			totalAccelerationStructureBufferSize,
			BufferAccessUsage::accelerationStructure,
			MemoryUsage::gpuOnly
		});

	const auto scratchBuffer = rhi_.createBuffer(BufferDescriptor{ totalScratchBufferSize, BufferAccessUsage::storage, MemoryUsage::gpuOnly });

	auto accelerationStructures = std::vector<vk::AccelerationStructureKHR>{};
	accelerationStructures.resize(descriptors.size());

	for (auto i = u32{}; i < descriptors.size(); i++)
	{
		const auto accelerationStructureCreateInfo = vk::AccelerationStructureCreateInfoKHR
		{
			.buffer = rhi_.bufferStorage_.get(accelerationStructureBuffer).buffer,
			.offset = accelerationStructureBufferOffsets[i],
			.size = buildSizes[i].accelerationStructureSize,
			.type = vk::AccelerationStructureTypeKHR::eBottomLevel
		};

		const auto result = rhi_.device_.createAccelerationStructureKHR(accelerationStructureCreateInfo);

		TOY_ASSERT(result.result == vk::Result::eSuccess);

		accelerationStructures[i] = result.value;
	}


	const auto scratchBufferDeviceAddress = rhi_.device_.getBufferAddress(vk::BufferDeviceAddressInfo
		{
				.buffer = rhi_.bufferStorage_.get(scratchBuffer).buffer
		});

	for (auto i = u32{}; i < descriptors.size(); i++)
	{
		buildInfos[i].dstAccelerationStructure = accelerationStructures[i];
		buildInfos[i].scratchData.deviceAddress = scratchBufferDeviceAddress + scratchBufferOffsets[i];
	}

	auto buildRanges = std::vector<vk::AccelerationStructureBuildRangeInfoKHR>{};
	buildRanges.resize(descriptors.size());

	auto buildRangeInfos = std::vector<vk::AccelerationStructureBuildRangeInfoKHR*>{};
	buildRangeInfos.resize(descriptors.size());

	for (auto i = u32{}; i < descriptors.size(); i++)
	{
		buildRanges[i] = vk::AccelerationStructureBuildRangeInfoKHR
		{
			.primitiveCount = descriptors[i].primitiveCount,
			.primitiveOffset = static_cast<u32>(descriptors[i].primitiveOffset * sizeof(float) * 3),
			.firstVertex = 0,//TODO: it could be easily adapted to per meschlet based tlas 
			.transformOffset = 0
		};
		buildRangeInfos[i] = &buildRanges[i];
	}

	commandBuffer_.buildAccelerationStructuresKHR(buildInfos, buildRangeInfos);


	return std::vector<Handle<AccelerationStructure>>();
}

struct RayTracingDrawData
{
	Handle<AccelerationStructure> objectAS;
	glm::mat4 transform;
	u32 materialIndex;
};


Handle<AccelerationStructure> VulkanCommandList::
buildAccelerationStructureInternal(
	const std::initializer_list<AccelerationStructureInstance>& instances)
{
	//TODO: scratch buffer can be reused also for TLAS build
	auto instancesHostData = std::vector<vk::AccelerationStructureInstanceKHR>{};
	instancesHostData.resize(instances.size());

	/*
	 *
	 *fill instance data,
	 *upload to a GPU resident buffer
	 *			- who defines residency place?
	 *			- instanceShaderBindingTableRecordOffset is more problematic, because it is strongly depend on used material
	 *get a buffer handler (vk::DeviceAddress)
	 *
	 *should transforms be managed internally or by user? 
	 *
	 *
	 *
	 */

	Handle<Buffer> instancesBuffer;
	const auto instancesData = rhi_.device_.getBufferAddress(
		vk::BufferDeviceAddressInfo
		{
			.buffer = rhi_.bufferStorage_.get(instancesBuffer).buffer
		});

	const auto accelerationStructureGeometry = vk::AccelerationStructureGeometryKHR
	{
		.geometryType = vk::GeometryTypeKHR::eInstances,
		.geometry =
		{
			.instances = vk::AccelerationStructureGeometryInstancesDataKHR
			{
				.arrayOfPointers = vk::Bool32{ false },
				.data = {instancesData}
			}
		},
		.flags = mapGeometryFlags(GeometryBehavior::none)
	};

	auto properties = vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceAccelerationStructurePropertiesKHR>{};
	rhi_.adapter_.getProperties2(&properties.get());

	const auto accelerationStructureProperties = properties.get<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();



	return {};
}

void VulkanCommandList::endInternal()
{
	//commandBuffer_.endDebugUtilsLabelEXT();
	const auto result = commandBuffer_.end();
	TOY_ASSERT(result == vk::Result::eSuccess);
	
}

void VulkanCommandList::beginInternal()
{
	const auto beginInfo = vk::CommandBufferBeginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};


	
	
	const auto result = commandBuffer_.begin(beginInfo);
	TOY_ASSERT(result == vk::Result::eSuccess);
	//const auto debugLabel = vk::DebugUtilsLabelEXT
	//{
	//	.pLabelName = "....",
	//	.color = std::array{0.5f,0.5f,0.5f,1.0f}
	//};
	//commandBuffer_.beginDebugUtilsLabelEXT(debugLabel);
}
