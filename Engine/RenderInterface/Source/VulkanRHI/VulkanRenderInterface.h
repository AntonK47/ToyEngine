#pragma once



#include <map>
#include <unordered_map>
#include <rigtorp/MPMCQueue.h>
#include <initializer_list>
#include <memory>
#include <memory_resource>

#include <Core.h>
#include <Hash.h>

#include "VulkanRHI/VulkanRenderInterfaceTypes.h"
#include "VulkanCommandList.h"
#include "VulkanRHI/Vulkan.h"
#include "Handle.h"

#include "RenderInterfaceBase.h"
#include "RenderInterfaceTypes.h"

#include "QueueType.h"
#include "ValidationCommon.h"
#include "SubmitDependency.h"
#include "SubmitBatch.h"


namespace toy::graphics::rhi
{
	class CommandList;
}

namespace toy::graphics::rhi::vulkan
{
	class VulkanRenderInterface : public RenderInterfaceBase
	{
	protected:
		auto initializeInternal(const RendererDescriptor& descriptor) -> void;
		auto deinitializeInternal() -> void;
		
		auto resizeBackbufferInternal(const u32 width, const u32 height) -> void;

		[[nodiscard]] auto acquireCommandListInternal(
			QueueType queueType,
			const UsageScope& usageScope) -> CommandList;

		[[nodiscard]] auto submitCommandListInternal(
			QueueType queueType,
			const RangeOf<CommandList> auto& commandLists,
			const RangeOf<SubmitDependency> auto& dependencies) -> SubmitBatch;

		auto nextFrameInternal() -> void;

		auto requestMemoryBudgetInternal() -> MemoryBudget;

		[[nodiscard]] auto acquireNextSwapchainImageInternal() -> SwapchainImage;

		auto presentInternal(const SubmitDependency& dependency) -> void;

		inline [[nodiscard]] auto getNativeBackendInternal() -> NativeBackend
		{
			return NativeBackend{ &nativeBackend_ };
		}

		auto beginDebugLabelInternal(
			const QueueType queueType,
			const DebugLabel& label) -> void;
		auto endDebugLabelInternal(const QueueType queueType) -> void;

		[[nodiscard]] auto createBindGroupLayoutInternal(
			const BindGroupDescriptor& descriptor, const DebugLabel& label) -> Handle<BindGroupLayout>;

		[[nodiscard]] auto allocateBindGroupInternal(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount,
			const UsageScope& scope,
			const DebugLabel& label) -> std::vector<Handle<BindGroup>>; //TODO: smallvector

		[[nodiscard]] auto createPipelineInternal(
			const GraphicsPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups,
			const std::vector<PushConstant>& pushConstants) -> Handle<
			Pipeline>;

		[[nodiscard]] auto createShaderModuleInternal(
			ShaderStage stage,
			const ShaderCode& code) -> Handle<ShaderModule>;

		auto resetDescriptorPoolsUntilFrame(const core::u32 frame) -> void;

		[[nodiscard]] auto createBufferInternal(
			const BufferDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label) -> Handle<Buffer>;

		auto destroyBufferInternal(const Handle<Buffer> handle) -> void;

		[[nodiscard]] auto createVirtualTextureInternal(
			const VirtualTextureDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label = DebugLabel{}) -> Handle<VirtualTexture>;


		[[nodiscard]] auto createSamplerInternal(
			const SamplerDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label) -> Handle<Sampler>;

		auto updateBindGroupInternal(
			const Handle<BindGroup>& bindGroup,
			const RangeOf<BindingDataMapping> auto& mappings) -> void;

		auto mapInternal(
			const Handle<Buffer>& buffer,
			void** data) -> void;

		auto unmapInternal(const Handle<Buffer>& buffer) -> void;

		[[nodiscard]] auto createPipelineInternal(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups,
			const std::vector<PushConstant>& pushConstants) -> Handle<
			Pipeline>;

		[[nodiscard]] auto createImageInternal(
			const ImageDescriptor& descriptor) -> Handle<Image>;
		[[nodiscard]] auto createImageViewInternal(
			const ImageViewDescriptor& descriptor) -> Handle<ImageView>;

		auto submitBatchesInternal(
			const QueueType queueType,
			const RangeOf<SubmitBatch> auto& batches) -> void;

		PerThreadCommandPoolData initializePerRenderThreadData();

		auto getMemoryRequirements() -> void;
		auto allocatePageMemoryInternal() -> void;

	private:

		vk::CommandBuffer getCommandBufferFromThread(QueueType queueType);


		std::unordered_map<QueueType, DeviceQueue> queues_;

		DeviceQueue presentQueue_{};
		vk::Semaphore readyToPresentSemaphore_;
		vk::Semaphore readyToRenderSemaphore_;
		core::u32 currentImageIndex_{};

		VmaAllocator allocator_{};
		vk::PhysicalDeviceMemoryProperties2 memoryProperties_{};

		VulkanNativeBackend nativeBackend_{};
		vk::Device device_;
		vk::Instance instance_;
		vk::PhysicalDevice adapter_;

		//UploadBufferRing uploadBuffer_{};

		static constexpr core::u32 maxCommandListsPerFrame_{ 10 };
		static constexpr core::u32 maxDeferredFrames_{ 3 };

		u32 lastTransferSignalValue{0};

		core::u32 currentFrame_{};
		std::thread::id renderThreadId_;

		std::vector<PerThreadCommandPoolData> perThreadData_{};

		std::unordered_map<std::thread::id, u32> workerIndexMap_{};


		std::vector<vk::Semaphore> timelineSemaphorePerFrame_{};

		std::map<QueueType, vk::Semaphore> timelineSemaphorePerQueue_{};

		vk::SurfaceKHR surface_;
		vk::SwapchainKHR swapchain_;

		static constexpr core::u32 swapchainImagesCount_ = 3;
		std::vector<Handle<ImageView>> swapchainImageViews_{};
		std::vector<Handle<Image>> swapchainImages_{};
		std::vector<vk::Fence> swapchainImageAfterPresentFences_{};



		std::unordered_map < core::u32, VulkanBindGroupLayout > bindGroupLayoutCache_{};

		LinearFrameAllocator<vk::DescriptorSet> bindGroupCache_{};

		std::array<std::vector<vk::DescriptorPool>, swapchainImagesCount_> descriptorPoolsPerFrame_;

		std::vector<vk::DescriptorPool> descriptorPoolsPersistent_;
		core::u32 lastFrameInUse_{};


		static constexpr core::u32 maxCommandListsPerSubmit_ = 10;
		static constexpr core::u32 maxSubmits_ = 100;


		PipelineCache graphicsPipelineCache_{};
		PipelineCache computePipelineCache_{};

		Pool<ShaderModule, VulkanShaderModule> shaderModuleStorage_{};
		Pool<Pipeline, VulkanPipeline> pipelineStorage_{};

		Pool<Buffer, VulkanBuffer> bufferStorage_{};
		Pool<Image, VulkanImage> imageStorage_{};

		Pool<ImageView, VulkanImageView> imageViewStorage_{};
		Pool<Sampler, VulkanSampler> samplerStorage_{};

		Pool<BindGroup, VulkanBindGroup> bindGroupStorage_{};
		Pool<BindGroup, VulkanBindGroup> persistentBindGroupStorage_{};//TODO:: bind groups should be removed manual

		struct VulkanTexture
		{
			vk::Image image;
		};

		struct TextureDescriptor
		{

		};

		[[nodiscard]] auto createTextureInternal(
			const TextureDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label = DebugLabel{}) -> Handle<Texture>;


		//Pool<Texture, VulkanTexture> 

		struct ResourceManager
		{
			VulkanRenderInterface& interface_;

			ResourceManager(VulkanRenderInterface& i) : interface_(i) {}

			struct TextureDescriptor
			{

			};

			[[nodiscard]] auto createTextureInternal(
				const TextureDescriptor& descriptor,
				[[maybe_unused]] const DebugLabel label = DebugLabel{}) -> Handle<Texture>;
		};

		friend ResourceManager;
		friend VulkanCommandList;

		//ResourceManager resourceManager_;

		/*[[nodiscard]]ResourceManager& resourceManager()
		{
			return resourceManager_;
		}*/
		/*
		 *
		 *I need two independent frame allocators, one on the Host site and other living on device
		 *
		 */
	};

	auto VulkanRenderInterface::submitCommandListInternal(
		const QueueType queueType,
		const RangeOf<CommandList> auto& commandLists,
		const RangeOf<SubmitDependency> auto& dependencies) -> SubmitBatch
	{
		TOY_ASSERT(!std::empty(commandLists));
		auto hasDependency = std::array{ false, false, false };
		auto maxValue = std::array{ u64{}, u64{}, u64{} };

		for (const auto& [queue, value] : dependencies)
		{
			hasDependency[static_cast<u32>(queue)] = true;
			maxValue[static_cast<u32>(queue)] = std::max(maxValue[static_cast<u32>(queue)], value);
		}

		auto graphicsCommandBuffers = std::vector<Submit>{};//TODO: smallvector

		auto submit = Submit
		{
			maxValue[static_cast<u32>(QueueType::graphics)],
			maxValue[static_cast<u32>(QueueType::asyncCompute)],
			maxValue[static_cast<u32>(QueueType::transfer)],
			u32{}
		};

		for (const auto& commandList : commandLists)
		{
			TOY_ASSERT(commandList.getQueueType() == queueType);
			submit.commandBuffers[submit.commandBuffersCount] = commandList.commandBuffer_;
			submit.commandBuffersCount++;
		}
		auto submitBatch = SubmitBatch{ queueType };
		submitBatch.batch_ = submit;

		return submitBatch;
	}

	auto VulkanRenderInterface::submitBatchesInternal(
		const QueueType queueType,
		const RangeOf<SubmitBatch> auto& batches) -> void
	{
		auto totalCommandBuffers = u32{};

		for (const auto& batch : batches)
		{
			totalCommandBuffers += batch.batch_.commandBuffersCount;
		}

		auto commandBufferSubmitInfos = std::vector<vk::CommandBufferSubmitInfo>{};//TODO: smallvector
		commandBufferSubmitInfos.resize(totalCommandBuffers);

		auto minGraphicsValue = u64{ std::numeric_limits<u64>::max() };
		auto minAsyncComputeValue = u64{ std::numeric_limits<u64>::max() };
		auto minTransferValue = u64{ std::numeric_limits<u64>::max() };

		auto value = u64{};

		auto index = u32{};
		for (const auto& batch : batches)
		{
			for (auto i = u32{}; i < batch.batch_.commandBuffersCount; i++)
			{

				minGraphicsValue = std::min(minGraphicsValue, batch.batch_.waitGraphicsValue);
				minAsyncComputeValue = std::min(minAsyncComputeValue, batch.batch_.waitAsyncComputeValue);
				minTransferValue = std::min(minTransferValue, batch.batch_.waitTransferValue);

				switch (queueType)
				{
				case QueueType::graphics:
					value = batch.batch_.waitGraphicsValue;
					break;
				case QueueType::asyncCompute:
					value = batch.batch_.waitAsyncComputeValue;
					break;
				case QueueType::transfer:
					value = batch.batch_.waitTransferValue;
					break;
				}

				commandBufferSubmitInfos[index].commandBuffer = batch.batch_.commandBuffers[i];
				index++;
			}
		}

		auto waitInfos = std::vector<vk::SemaphoreSubmitInfo>{};//TODO: smallvector
		waitInfos.reserve(3);

		if (minGraphicsValue != 0)
		{
			waitInfos.push_back
			(
				vk::SemaphoreSubmitInfo
				{
					.semaphore = timelineSemaphorePerQueue_[QueueType::graphics],
					.value = minGraphicsValue,
					.stageMask = vk::PipelineStageFlagBits2::eAllCommands, //TODO: select something smarter
					.deviceIndex = 0
				}
			);
		}

		if (minAsyncComputeValue != 0)
		{
			waitInfos.push_back
			(
				vk::SemaphoreSubmitInfo
				{
					.semaphore = timelineSemaphorePerQueue_[QueueType::asyncCompute],
					.value = minAsyncComputeValue,
					.stageMask = vk::PipelineStageFlagBits2::eAllCommands, //TODO: select something smarter
					.deviceIndex = 0
				}
			);
		}

		if (minTransferValue != 0)
		{
			waitInfos.push_back
			(
				vk::SemaphoreSubmitInfo
				{
					.semaphore = timelineSemaphorePerQueue_[QueueType::transfer],
					.value = minTransferValue,
					.stageMask = vk::PipelineStageFlagBits2::eAllCommands, //TODO: select something smarter
					.deviceIndex = 0
				}
			);
		}

		//TODO: Don't like it
		if(lastTransferSignalValue >= value)
		{
			value = lastTransferSignalValue;
		}

		lastTransferSignalValue++;

		const auto signalInfo = vk::SemaphoreSubmitInfo
		{
			.semaphore = timelineSemaphorePerQueue_[queueType],
			.value = value + 1,
			.stageMask = vk::PipelineStageFlagBits2::eAllCommands, //TODO: select something smarter
			.deviceIndex = 0
		};

		const auto submitInfo = vk::SubmitInfo2
		{
			.waitSemaphoreInfoCount = static_cast<u32>(std::size(waitInfos)),
			.pWaitSemaphoreInfos = std::data(waitInfos),
			.commandBufferInfoCount = static_cast<u32>(std::size(commandBufferSubmitInfos)),
			.pCommandBufferInfos = std::data(commandBufferSubmitInfos),
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalInfo
		};

		/*const auto& fence = swapchainImageAfterPresentFences_[currentFrame_ % maxDeferredFrames_];
		auto result = device_.resetFences(1, &fence);*/
		//TOY_ASSERT(result == vk::Result::eSuccess);
		const auto queue = queues_[queueType].queue;
		{
			auto result = queue.submit2(1, &submitInfo, nullptr);
			TOY_ASSERT(result == vk::Result::eSuccess);
		}
		/*
		auto result = device_.waitIdle();
		TOY_ASSERT(result == vk::Result::eSuccess);*/
	}

	auto VulkanRenderInterface::updateBindGroupInternal(
		const Handle<BindGroup>& bindGroup,
		const RangeOf<BindingDataMapping> auto& mappings) -> void
	{
		auto vulkanBindGroup = VulkanBindGroup{};
		//TODO: do something clever
		if (persistentBindGroupStorage_.contains(bindGroup))
		{
			vulkanBindGroup = persistentBindGroupStorage_.get(bindGroup);
		}
		else
		{
			vulkanBindGroup = bindGroupStorage_.get(bindGroup);
		}

		auto descriptorWrites = std::vector<vk::WriteDescriptorSet>{}; //TODO: smallvector
		descriptorWrites.reserve(mappings.size());

		auto descriptorInfos = std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>>{};//TODO: smallvector
		descriptorInfos.resize(mappings.size());

		auto i = u32{};
		for (const auto& binding : mappings)
		{
			std::visit(
				Overloaded
				{
					[&](const CBV& cbv)
					{
						const auto& bufferView = std::get<CBV>(binding.view);
						const auto& vulkanBuffer = bufferStorage_.get(bufferView.bufferView.buffer);
						const auto descriptorBufferInfo = vk::DescriptorBufferInfo
						{
							.buffer = vulkanBuffer.buffer,
							.offset = bufferView.bufferView.offset,
							.range = bufferView.bufferView.size
						};
						descriptorInfos[i] = descriptorBufferInfo;
					},
					[&](const UAV& uav)
					{
						const auto& bufferView = std::get<UAV>(binding.view);
						const auto& vulkanBuffer = bufferStorage_.get(bufferView.bufferView.buffer);
						const auto descriptorBufferInfo = vk::DescriptorBufferInfo
						{
							.buffer = vulkanBuffer.buffer,
							.offset = bufferView.bufferView.offset,
							.range = bufferView.bufferView.size
						};
						descriptorInfos[i] = descriptorBufferInfo;
					},
					[&](const Texture2DSRV& srv)
					{
						const auto& imageSrv = std::get<Texture2DSRV>(binding.view);
						const auto& imageView = imageViewStorage_.get(imageSrv.imageView);
						const auto& descriptorImageInfo = vk::DescriptorImageInfo
						{
							.imageView = imageView.imageView,
							.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
						};
						descriptorInfos[i] = descriptorImageInfo;
					},
						[&](const SamplerSRV& srv)
					{
						const auto& samplerSrv = std::get<SamplerSRV>(binding.view);
						const auto& sampler = samplerStorage_.get(samplerSrv.sampler);
						const auto& descriptorImageInfo = vk::DescriptorImageInfo
						{
							.sampler = sampler.sampler
						};
						descriptorInfos[i] = descriptorImageInfo;
					}
				},
				binding.view);
			i++;
		}

		i = 0;

		for (const auto& binding : mappings)
		{
			if (std::holds_alternative<CBV>(binding.view))
			{
				TOY_ASSERT(std::holds_alternative<CBV>(binding.view));

				const auto write = vk::WriteDescriptorSet
				{
					.dstSet = vulkanBindGroup.descriptorSet,
					.dstBinding = binding.binding,
					.dstArrayElement = binding.arrayElement,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer, //because of CBV type, TODO: derive it properly!
					.pBufferInfo = &std::get<vk::DescriptorBufferInfo>(descriptorInfos[i])
				};

				descriptorWrites.push_back(write);
			}
			if (std::holds_alternative<UAV>(binding.view))
			{
				TOY_ASSERT(std::holds_alternative<UAV>(binding.view));

				const auto write = vk::WriteDescriptorSet
				{
					.dstSet = vulkanBindGroup.descriptorSet,
					.dstBinding = binding.binding,
					.dstArrayElement = binding.arrayElement,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eStorageBuffer, //because of UAV type, TODO: derive it properly!
					.pBufferInfo = &std::get<vk::DescriptorBufferInfo>(descriptorInfos[i])
				};

				descriptorWrites.push_back(write);
			}
			if (std::holds_alternative<Texture2DSRV>(binding.view))
			{
				TOY_ASSERT(std::holds_alternative<Texture2DSRV>(binding.view));

				const auto write = vk::WriteDescriptorSet
				{
					.dstSet = vulkanBindGroup.descriptorSet,
					.dstBinding = binding.binding,
					.dstArrayElement = binding.arrayElement,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eSampledImage,
					.pImageInfo = &std::get<vk::DescriptorImageInfo>(descriptorInfos[i])
				};

				descriptorWrites.push_back(write);
			}
			if (std::holds_alternative<SamplerSRV>(binding.view))
			{
				TOY_ASSERT(std::holds_alternative<SamplerSRV>(binding.view));

				const auto write = vk::WriteDescriptorSet
				{
					.dstSet = vulkanBindGroup.descriptorSet,
					.dstBinding = binding.binding,
					.dstArrayElement = binding.arrayElement,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eSampler,
					.pImageInfo = &std::get<vk::DescriptorImageInfo>(descriptorInfos[i])
				};

				descriptorWrites.push_back(write);
			}
			i++;
		}

		device_.updateDescriptorSets(static_cast<u32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
