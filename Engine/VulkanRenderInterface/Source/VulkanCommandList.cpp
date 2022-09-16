#include "VulkanCommandList.h"

namespace
{
	vk::ImageView getImageView(const Handle<Texture>& handle)
	{
		return vk::ImageView{};
	}

	vk::ResolveModeFlagBits mapResolve(ResolveMode resolveMode)
	{
		return vk::ResolveModeFlagBits::eNone;
	}

	vk::AttachmentLoadOp mapLoadOp(LoadOperation load)
	{
		return vk::AttachmentLoadOp::eLoad;
	}

	vk::ClearValue mapClearValue(const ColorClear& colorClear)
	{
		return vk::ClearValue{};
	}

	vk::AttachmentStoreOp mapStoreOp(StoreOperation store)
	{
		return vk::AttachmentStoreOp::eStore;
	}

	bool hasDepthAttachment(const RenderingDescriptor& description)
	{
		return false;
	}

	bool hasStencilAttachment(const RenderingDescriptor& description)
	{
		return false;
	}

	vk::ShaderModule createShaderModule(const vk::Device device)
	{
		const auto moduleCreateInfo = vk::ShaderModuleCreateInfo
		{

		};
		return device.createShaderModule(moduleCreateInfo);
	}

	vk::Pipeline createGraphicsPipeline(const vk::Device device)
	{
		auto vertexModule = vk::ShaderModule{};
		auto fragmentModule = vk::ShaderModule{};

		const auto stages = std::array
		{
			vk::PipelineShaderStageCreateInfo
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = vertexModule,
				.pName = "main",
				.pSpecializationInfo = nullptr
			},
			vk::PipelineShaderStageCreateInfo
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = fragmentModule,
				.pName = "main",
				.pSpecializationInfo = nullptr
			}
		};

		const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo
		{
			.topology = vk::PrimitiveTopology::eTriangleList
		};

		const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo
		{

		};

		const auto dynamicStates = std::array
		{
			vk::DynamicState::eDepthTestEnable
		};

		const auto dynamicState = vk::PipelineDynamicStateCreateInfo
		{
			.dynamicStateCount = dynamicStates.size(),
			.pDynamicStates = dynamicStates.data()
		};


		const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo
		{

		};

		const auto pipelineLayout = device.createPipelineLayout(layoutCreateInfo);

		const auto pipelineCreateInfo = vk::StructureChain
		{
			vk::GraphicsPipelineCreateInfo
			{
				.stageCount = stages.size(),
				.pStages = stages.data(),
				.pVertexInputState = nullptr,
				.pInputAssemblyState = &inputAssemblyState,
				.pTessellationState = nullptr,
				.pViewportState = nullptr,
				.pRasterizationState = &rasterizationState,
				.pMultisampleState = nullptr,
				.pDepthStencilState = nullptr,
				.pColorBlendState = nullptr,
				.pDynamicState = &dynamicState,
				.layout = pipelineLayout,
			},
			vk::PipelineRenderingCreateInfo
			{

			}

		};

		auto pipelineCache = vk::PipelineCache{};

		return device.createGraphicsPipeline(pipelineCache, pipelineCreateInfo.get()).value;
	}
}

void api::vulkan::VulkanCommandList::beginRendering(RenderingDescriptor description)
{
	const auto containsSecondaryBuffers = false;
	auto flags = vk::RenderingFlags{};
	if(containsSecondaryBuffers)
	{
		flags |= vk::RenderingFlagBits::eContentsSecondaryCommandBuffers;
	}
			
	auto colorAttachments = std::vector<vk::RenderingAttachmentInfo>(description.colorRenderTargets.size());

	for (u32 i{}; i<description.colorRenderTargets.size(); i++)
	{
		const auto& colorRenderTarget = description.colorRenderTargets[i];

		colorAttachments[i] = vk::RenderingAttachmentInfo
		{
			.imageView = getImageView(colorRenderTarget.texture),
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.resolveMode = mapResolve(colorRenderTarget.resolveMode),
			.loadOp = mapLoadOp(colorRenderTarget.load),
			.storeOp = mapStoreOp(colorRenderTarget.store),
			.clearValue = mapClearValue(colorRenderTarget.clearValue)
		};
	}

	auto depthAttachment = vk::RenderingAttachmentInfo{};

	if(hasDepthAttachment(description))
	{
		depthAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = getImageView(description.depthRenderTarget.texture),
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.resolveMode = mapResolve(description.depthRenderTarget.resolveMode),
			.loadOp = mapLoadOp(description.depthRenderTarget.load),
			.storeOp = mapStoreOp(description.depthRenderTarget.store),
			.clearValue = mapClearValue(description.depthRenderTarget.clearValue)
		};
	}

	auto stencilAttachment = vk::RenderingAttachmentInfo{};
	if(hasStencilAttachment(description))
	{
		stencilAttachment = vk::RenderingAttachmentInfo
		{
			.imageView = getImageView(description.stencilRenderTarget.texture),
			.imageLayout = vk::ImageLayout::eStencilAttachmentOptimal,
			.resolveMode = mapResolve(description.stencilRenderTarget.resolveMode),
			.loadOp = mapLoadOp(description.stencilRenderTarget.load),
			.storeOp = mapStoreOp(description.stencilRenderTarget.store),
			.clearValue = mapClearValue(description.stencilRenderTarget.clearValue)
		};
	}

	const auto renderingInfo = vk::RenderingInfo
	{
		.flags = flags,
		.renderArea = vk::Rect2D{ },
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = static_cast<u32>(colorAttachments.size()),
		.pColorAttachments = colorAttachments.data(),
		.pDepthAttachment = hasDepthAttachment(description)?&depthAttachment:nullptr,
		.pStencilAttachment = hasStencilAttachment(description)?&stencilAttachment:nullptr
	};

	cmd_.beginRendering(renderingInfo);
}

void api::vulkan::VulkanCommandList::endRendering()
{
	cmd_.endRendering();
}
