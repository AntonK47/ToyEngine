#include "VulkanCommandList.h"
#include "VulkanMappings.h"

using namespace toy::renderer;
using namespace api::vulkan;

namespace
{
	vk::ResolveModeFlagBits mapResolve(ResolveMode resolveMode)
	{
		return vk::ResolveModeFlagBits::eNone;
	}

	vk::AttachmentLoadOp mapLoadOp(LoadOperation load)
	{
		return vk::AttachmentLoadOp::eClear;
	}

	vk::ClearValue mapClearValue(const ColorClear& colorClear)
	{
		return vk::ClearValue{ vk::ClearColorValue{ std::array<float,4>{colorClear.r,colorClear.g,colorClear.b,colorClear.a} } };
	}

	vk::ClearValue mapDepthClearValue(const DepthClear& colorClear)
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

	vk::AttachmentStoreOp mapStoreOp(StoreOperation store)
	{
		//TODO: it should have proper mapping now
		return vk::AttachmentStoreOp::eStore;
	}

	bool hasDepthAttachment(const RenderingDescriptor& descriptor)
	{
		return true;//TODO:?????
	}

	bool hasStencilAttachment(const RenderingDescriptor& descriptor)
	{
		return false;
	}

	vk::GeometryFlagsKHR mapGeometryFlags(GeometryBehavior behavier)
	{
		return vk::GeometryFlagsKHR{};
	}
}

VulkanCommandList::~VulkanCommandList()
{
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
			barrier.image = renderInterface_->imageStorage_.get(imageBarrierDescriptor.image).image;
			barrier.subresourceRange = vk::ImageSubresourceRange
			{
				mapViewAspect(imageBarrierDescriptor.aspect), 0, 1, 0, 1
			};

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
			default:

				barrier.newLayout = vk::ImageLayout::eUndefined;
				barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
				break;
			}
			
			imageBarriers.push_back(barrier);
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

	cmd_.pipelineBarrier2(dependency);

}

void VulkanCommandList::beginRenderingInternal(const RenderingDescriptor& descriptor, const RenderArea& area)
{
	const auto containsSecondaryBuffers = false;
	auto flags = vk::RenderingFlags{};
	if(containsSecondaryBuffers)
	{
		flags |= vk::RenderingFlagBits::eContentsSecondaryCommandBuffers;
	}
			
	auto colorAttachments = std::vector<vk::RenderingAttachmentInfo>(descriptor.colorRenderTargets.size());

	for (u32 i{}; i< descriptor.colorRenderTargets.size(); i++)
	{
		const auto& colorRenderTarget = descriptor.colorRenderTargets[i];

		auto imageView = renderInterface_->imageViewStorage_.get(descriptor.colorRenderTargets[i].imageView).imageView;

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
		auto imageView = renderInterface_->imageViewStorage_.get(descriptor.depthRenderTarget.imageView).imageView;
		depthAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.resolveMode = mapResolve(descriptor.depthRenderTarget.resolveMode),
			.loadOp = mapLoadOp(descriptor.depthRenderTarget.load),
			.storeOp = mapStoreOp(descriptor.depthRenderTarget.store),
			.clearValue = mapDepthClearValue(std::get<DepthClear>(descriptor.depthRenderTarget.clearValue))
		};
	}

	auto stencilAttachment = vk::RenderingAttachmentInfo{};
	if(hasStencilAttachment(descriptor))
	{
		auto imageView = renderInterface_->imageViewStorage_.get(descriptor.stencilRenderTarget.imageView).imageView;
		stencilAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eStencilAttachmentOptimal,
			.resolveMode = mapResolve(descriptor.stencilRenderTarget.resolveMode),
			.loadOp = mapLoadOp(descriptor.stencilRenderTarget.load),
			.storeOp = mapStoreOp(descriptor.stencilRenderTarget.store),
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

	cmd_.beginRendering(renderingInfo);
}

void VulkanCommandList::endRenderingInternal()
{
	cmd_.endRendering();
}

void VulkanCommandList::drawInternal(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
{
	
	cmd_.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::bindPipelineInternal(const Handle<Pipeline>& pipeline)
{
	const auto& vulkanPipeline = renderInterface_->pipelineStorage_.get(pipeline);
	currentPipeline_ = vulkanPipeline;
	cmd_.bindPipeline(currentPipeline_.bindPoint, currentPipeline_.pipeline);
}

void VulkanCommandList::setScissorInternal(const Scissor& scissor)
{
	const auto vulkanScissor = vk::Rect2D{ {scissor.x,scissor.y},{scissor.width, scissor.height}};
	
	cmd_.setScissor(0, 1, &vulkanScissor);
}

void VulkanCommandList::setViewportInternal(
	const Viewport& viewport)
{
	//TODO: flip viewport???
	const auto vulkanViewport = vk::Viewport{ viewport.x,viewport.y + viewport.height,viewport.width,-viewport.height,0.0,1.0 };

	//TODO:: Whats about depth value?

	cmd_.setViewport(0, 1, &vulkanViewport);
}

void VulkanCommandList::bindGroupInternal(
	const u32 set,
	const Handle<BindGroup>& handle)
{
	auto vulkanBindGroup = VulkanBindGroup{};
	//TODO: do something clever
	if (renderInterface_->persistentBindGroupStorage_.contains(handle))
	{
		vulkanBindGroup = renderInterface_->persistentBindGroupStorage_.get(handle);
	}
	else
	{
		vulkanBindGroup = renderInterface_->bindGroupStorage_.get(handle);
	}
	//const auto& vulkanBindGroup = renderInterface_->bindGroupStorage_.get(handle);
	const auto& vulkanPipeline = currentPipeline_;


	cmd_.bindDescriptorSets(vulkanPipeline.bindPoint, vulkanPipeline.layout, set, 1, &vulkanBindGroup.descriptorSet, 0, nullptr);
}

std::vector<Handle<AccelerationStructure>> VulkanCommandList::
buildAccelerationStructureInternal(const TriangleGeometry& geometry,
	const std::vector<AccelerationStructureDescriptor>& descriptors)
{
	const auto vertexData = renderInterface_->device_.getBufferAddress(
		vk::BufferDeviceAddressInfo
		{
			.buffer = renderInterface_->bufferStorage_.get(geometry.vertexBuffer).buffer
		});

	const auto indexData = renderInterface_->device_.getBufferAddress(
		vk::BufferDeviceAddressInfo
		{
			.buffer = renderInterface_->bufferStorage_.get(geometry.indexBuffer).buffer
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
	renderInterface_->adapter_.getProperties2(&properties.get());

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

		buildSizes[i] = renderInterface_->device_.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfos[i], maxPrimitiveCount);


		scratchBufferOffsets[i] = totalScratchBufferSize;
		accelerationStructureBufferOffsets[i] = totalAccelerationStructureBufferSize;

		totalAccelerationStructureBufferSize += buildSizes[i].accelerationStructureSize + 256 - buildSizes[i].accelerationStructureSize % 256;
		totalScratchBufferSize += buildSizes[i].buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment - buildSizes[i].buildScratchSize % accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
	}

	const auto accelerationStructureBuffer = renderInterface_->createBuffer(BufferDescriptor{ totalAccelerationStructureBufferSize, BufferAccessUsage::accelerationStructure, MemoryUsage::gpuOnly });

	const auto scratchBuffer = renderInterface_->createBuffer(BufferDescriptor{ totalScratchBufferSize, BufferAccessUsage::storage, MemoryUsage::gpuOnly });

	auto accelerationStructures = std::vector<vk::AccelerationStructureKHR>{};
	accelerationStructures.resize(descriptors.size());

	for (auto i = u32{}; i < descriptors.size(); i++)
	{
		const auto accelerationStructureCreateInfo = vk::AccelerationStructureCreateInfoKHR
		{
			.buffer = renderInterface_->bufferStorage_.get(accelerationStructureBuffer).buffer,
			.offset = accelerationStructureBufferOffsets[i],
			.size = buildSizes[i].accelerationStructureSize,
			.type = vk::AccelerationStructureTypeKHR::eBottomLevel
		};

		const auto result = renderInterface_->device_.createAccelerationStructureKHR(accelerationStructureCreateInfo);

		TOY_ASSERT(result.result == vk::Result::eSuccess);

		accelerationStructures[i] = result.value;
	}


	const auto scratchBufferDeviceAddress = renderInterface_->device_.getBufferAddress(vk::BufferDeviceAddressInfo
		{
				.buffer = renderInterface_->bufferStorage_.get(scratchBuffer).buffer
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
			.primitiveOffset = descriptors[i].primitiveOffset*sizeof(float)*3,
			.firstVertex = 0,//TODO: it could be easily adapted to per meschlet based tlas 
			.transformOffset = 0
		};
		buildRangeInfos[i] = &buildRanges[i];
	}

	cmd_.buildAccelerationStructuresKHR(buildInfos, buildRangeInfos);


	return std::vector<Handle<AccelerationStructure>>();
}



VulkanCommandList::VulkanCommandList(
	VulkanRenderInterface& parent,
	const vk::CommandBuffer commandBuffer,
	const vk::CommandBufferLevel level,
	const QueueType ownedQueueType) :
	CommandList(ownedQueueType),
	renderInterface_(&parent),
	cmd_(commandBuffer),
	level_(level)
{}
