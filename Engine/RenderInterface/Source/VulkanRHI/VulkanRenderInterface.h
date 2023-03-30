#pragma once



#include <map>
#include <mutex>
#include <unordered_map>
#include <rigtorp/MPMCQueue.h>
#include <initializer_list>
#include <memory>
#include <memory_resource>

#include <Core.h>
#include <Hash.h>

#include "VulkanRHI/Vulkan.h"
#include "Handle.h"

#include "RenderInterfaceBase.h"
#include "RenderInterfaceTypes.h"

#include "QueueType.h"

#include "ValidationCommon.h"

#include "SubmitDependency.h"

#include "VulkanRHI/VulkanRenderInterfaceTypes.h"

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


		[[nodiscard]] auto acquireCommandListInternal(QueueType queueType, const WorkerThreadId& workerId, const UsageScope& usageScope) -> CommandList;

		auto submitCommandListInternal(
			const CommandList& commandList) -> void;

		[[nodiscard]] auto submitCommandListInternal(
			QueueType queueType,
			const std::initializer_list<toy::graphics::rhi::CommandList>& commandLists,
			const std::initializer_list<SubmitDependency>& dependencies) -> SubmitBatch;

		auto nextFrameInternal() -> void;

		[[nodiscard]] auto acquireNextSwapchainImageInternal() -> SwapchainImage;

		auto presentInternal(const SubmitDependency& dependency) -> void;

		[[nodiscard]] auto getNativeBackendInternal() -> NativeBackend
		{
			return NativeBackend{ &nativeBackend_ };
		}


		auto beginDebugLabelInternal(const QueueType queueType, const DebugLabel& label) -> void;
		auto endDebugLabelInternal(const QueueType queueType) -> void;

		[[nodiscard]] auto createBindGroupLayoutInternal(
			const BindGroupDescriptor& descriptor) -> Handle<BindGroupLayout>;

		[[nodiscard]] auto allocateBindGroupInternal(
			const Handle<BindGroupLayout>& bindGroupLayout,
			u32 bindGroupCount,
			const UsageScope& scope) -> std::vector<Handle<BindGroup>>; //TODO: smallvector

		[[nodiscard]] auto createPipelineInternal(
			const GraphicsPipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups,
			const std::vector<PushConstant>& pushConstants) -> Handle<
			Pipeline>;

		[[nodiscard]] auto createShaderModuleInternal(
			ShaderStage stage,
			const ShaderCode& code) -> Handle<ShaderModule>;

		void resetDescriptorPoolsUntilFrame(const core::u32 frame);

		[[nodiscard]] auto createBufferInternal(
			const BufferDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label) -> Handle<Buffer>;
		
		[[nodiscard]] auto createVirtualTextureInternal(
			const VirtualTextureDescriptor& descriptor,
			[[maybe_unused]] const DebugLabel label = DebugLabel{}) -> Handle<VirtualTexture>;

		
		[[nodiscard]] auto createSamplerInternal(const SamplerDescriptor& descriptor, [[maybe_unused]] const DebugLabel label) -> Handle<Sampler>;

		auto updateBindGroupInternal(const Handle<BindGroup>& bindGroup,
		                             const std::initializer_list<
			                             BindingDataMapping>& mappings) -> void;

		auto mapInternal(const Handle<Buffer>& buffer, void** data) -> void;

		[[nodiscard]] auto createPipelineInternal(
			const ComputePipelineDescriptor& descriptor,
			const std::vector<SetBindGroupMapping>& bindGroups,
			const std::vector<PushConstant>& pushConstants) -> Handle<
			Pipeline>;

		[[nodiscard]] auto createImageInternal(
			const ImageDescriptor& descriptor) -> Handle<Image>;
		[[nodiscard]] auto createImageViewInternal(
			const ImageViewDescriptor& descriptor) -> Handle<ImageView>;


		void submitBatchesInternal(const QueueType queueType, const std::initializer_list<SubmitBatch>& batches);

		PerThreadCommandPoolData initializePerRenderThreadData();

		auto getMemoryRequirenments() -> void;
		auto allocatePageMemoryInternal() -> void;

	private:

		vk::CommandBuffer getCommandBufferFromThread(const WorkerThreadId& workerId, QueueType queueType);

		
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

		static constexpr core::u32 maxCommandListsPerFrame_{10};
		static constexpr core::u32 maxDeferredFrames_{3};


		core::u32 currentFrame_{};
		std::thread::id renderThreadId_;

		std::vector<PerThreadCommandPoolData> perThreadData_{};


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

			ResourceManager(VulkanRenderInterface& i) : interface_(i){}

			struct TextureDescriptor 
			{

			};

			[[nodiscard]] auto createTextureInternal(
				const TextureDescriptor& descriptor,
				[[maybe_unused]] const DebugLabel label = DebugLabel{}) -> Handle<Texture>;
		};

		friend ResourceManager;
		friend VulkanCommandList;

		//ResourceManager resouceManager_;

		/*[[nodiscard]]ResourceManager& resouceManager()
		{
			return resouceManager_;
		}*/
		/*
		 *
		 *I need two independent frame allocators, one on the Host site and other living on device
		 *
		 */
	};
}
