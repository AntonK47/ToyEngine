#include "VulkanRenderInterface.h"

#include <iostream>
#include <map>
#include <vector>
#include <VulkanCommandList.h>
#include <Windows.h>
#include <vulkan/vulkan_profiles.hpp>

#include "g3log/g3log.hpp"
const LEVELS VULKAN_VALIDATION_ERROR{ WARNING.value + 1, {"VULKAN_VALIDATION_ERROR_LEVEL"} };
using namespace toy::renderer;
using namespace api::vulkan;
namespace
{
#define MAP_FLAG_BIT(srcFlag, srcFlagBit, dstFlag, dstFlagBit) if (srcFlag.containBit(srcFlagBit)) { dstFlag |= dstFlagBit; }

    vk::DescriptorType mapDescriptorType(BindingType type)
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
    }

    PerThreadCommandPoolData createPerThreadCommandPoolData(vk::Device device,
                                                            vk::CommandBufferLevel level,
                                                            u32 maxDeferredFrames,
                                                            u32 graphicsIndex,
                                                            u32 asyncComputeIndex,
                                                            u32 transferIndex,
                                                            u32 graphicsCommandListPerFrame = 0,
                                                            u32 asyncComputeCommandListPerFrame = 0,
                                                            u32 transferCommandListPerFrame = 0
    )
    {
        auto graphicsPerFrame = std::vector<PerFrameCommandPoolData>(maxDeferredFrames);
        auto asyncComputePerFrame = std::vector <PerFrameCommandPoolData>(maxDeferredFrames);
        auto transferPerFrame = std::vector <PerFrameCommandPoolData>(maxDeferredFrames);
        for(u32 i{}; i < maxDeferredFrames; i++)
        {
	        if(graphicsCommandListPerFrame != 0)
	        {
                const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
                {
                    .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                    .queueFamilyIndex = graphicsIndex
                };

                graphicsPerFrame[i].commandPool = device.createCommandPool(commandPoolCreateInfo).value;

                const auto allocateInfo = vk::CommandBufferAllocateInfo
                {
                    .commandPool = graphicsPerFrame[i].commandPool,
                    .level = level,
                    .commandBufferCount = graphicsCommandListPerFrame
                };
                graphicsPerFrame[i].commandBuffers = device.allocateCommandBuffers(allocateInfo).value;
	        }
            if (asyncComputeCommandListPerFrame != 0)
            {
                const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
                {
                    .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                    .queueFamilyIndex = asyncComputeIndex
                };

                asyncComputePerFrame[i].commandPool = device.createCommandPool(commandPoolCreateInfo).value;

                const auto allocateInfo = vk::CommandBufferAllocateInfo
                {
                    .commandPool = asyncComputePerFrame[i].commandPool,
                    .level = level,
                    .commandBufferCount = asyncComputeCommandListPerFrame
                };
                asyncComputePerFrame[i].commandBuffers = device.allocateCommandBuffers(allocateInfo).value;
            }
            if (transferCommandListPerFrame != 0)
            {
                const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
                {
                    .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                    .queueFamilyIndex = transferIndex
                };

                transferPerFrame[i].commandPool = device.createCommandPool(commandPoolCreateInfo).value;

                const auto allocateInfo = vk::CommandBufferAllocateInfo
                {
                    .commandPool = transferPerFrame[i].commandPool,
                    .level = level,
                    .commandBufferCount = transferCommandListPerFrame
                };
                transferPerFrame[i].commandBuffers = device.allocateCommandBuffers(allocateInfo).value;
            }
        }


        PerThreadCommandPoolData perThreadData
        {
            .level = level
        };
        if(graphicsCommandListPerFrame!=0)
        {
            perThreadData.perQueueType[QueueType::graphics] = graphicsPerFrame;
        }
        if (asyncComputeCommandListPerFrame != 0)
        {
            perThreadData.perQueueType[QueueType::asyncCompute] = asyncComputePerFrame;
        }
        if (transferCommandListPerFrame != 0)
        {
            perThreadData.perQueueType[QueueType::transfer] = transferPerFrame;
        }

        return perThreadData;
    }


    struct Features
    {

    };

    VkBool32 debugCallback(  // NOLINT(clang-diagnostic-unused-function)
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void* pUserData)
    {
        switch (static_cast<uint32_t>(pCallbackData->messageIdNumber))
        {
        case 0x822806fa://we use debug utils 
        case 0xe9bdce0a://because we are using vkGetPhysicalDeviceSurfacePresentModes2EXT()
        //case 0x48a09f6c://temporally
            return vk::Bool32{ false };
        default:;
        }

        constexpr auto defaultColorSequence = "\033[0m";
        // ReSharper disable once CppTooWideScope
        constexpr auto errorColorSequence = "\033[31m";
        // ReSharper disable once CppTooWideScope
        constexpr auto warningColorSequence = "\033[33m";
        // ReSharper disable once CppTooWideScope
        [[maybe_unused]] constexpr auto infoColorSequence = "\033[32m";


        //TODO: logging of a verbose and info messages should be moved to an another logger, maybe in a file logger.
        switch (static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) {

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:  
            LOG(WARNING) << pCallbackData->pMessage;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            LOG(INFO) << pCallbackData->pMessage;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            LOG(WARNING) << pCallbackData->pMessage;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            LOG(VULKAN_VALIDATION_ERROR) << pCallbackData->pMessage;
            break;
        }
        

        return vk::Bool32{ true };
    }

    using QueueIndex = uint32_t;
    QueueIndex getQueueFamilyIndex(const vk::PhysicalDevice adapter, const vk::QueueFlags flags)
    {
        const auto queueFamilyProperties = adapter.getQueueFamilyProperties2();
        for (uint32_t i{}; i < queueFamilyProperties.size(); i++)
        {
            const auto& queueFamilyProperty = queueFamilyProperties[i].queueFamilyProperties;
            if ((queueFamilyProperty.queueFlags & flags) == flags)
            {
                return i;
            }
        }
        return 0;
    }

    vk::Device createDevice(vk::PhysicalDevice adapter, const std::vector<std::reference_wrapper<api::vulkan::DeviceQueue>>& queues, const Features& requiredFeatures)
    {
        std::cout << "device name: " << adapter.getProperties2().properties.deviceName << std::endl;

        auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>{};

        auto familyIndexQueueMap = std::map<uint32_t, std::vector<std::reference_wrapper<api::vulkan::DeviceQueue>>>{};

        for (auto& queue : queues)
        {
            familyIndexQueueMap[queue.get().familyIndex].push_back(queue);
        }
        auto priorities = std::vector<float>{};
        for (auto& [index, queues] : familyIndexQueueMap)
        {
            
            priorities.resize(queues.size());

            for(uint32_t i{}; i < queues.size(); i++)
            {
	            queues[i].get().queueIndex = i;
                priorities[i] = queues[i].get().priority;
            }

            queueCreateInfos.push_back(
                vk::DeviceQueueCreateInfo
                {
                    .queueFamilyIndex = index,
                    .queueCount = static_cast<uint32_t>(queues.size()),
                    .pQueuePriorities = priorities.data()
                }
            );
        }

        auto extensions = std::vector
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        const auto features = vk::StructureChain
        {
            vk::PhysicalDeviceFeatures2
            {
                .features = vk::PhysicalDeviceFeatures
                {
                    .shaderInt64 = vk::Bool32{true}
                }
            },
            vk::PhysicalDeviceVulkan11Features
            {

            },
            vk::PhysicalDeviceVulkan12Features
            {
            	.shaderBufferInt64Atomics = vk::Bool32{true},

                .shaderUniformBufferArrayNonUniformIndexing = vk::Bool32{true},
                .shaderSampledImageArrayNonUniformIndexing = vk::Bool32{true},
                .shaderStorageBufferArrayNonUniformIndexing = vk::Bool32{true},
                .shaderStorageImageArrayNonUniformIndexing = vk::Bool32{ true },
                .shaderInputAttachmentArrayNonUniformIndexing = vk::Bool32{ true },
                .shaderUniformTexelBufferArrayNonUniformIndexing = vk::Bool32{ true },
                .shaderStorageTexelBufferArrayNonUniformIndexing = vk::Bool32{ true },
                .descriptorBindingUniformBufferUpdateAfterBind = vk::Bool32{ true },
                .descriptorBindingSampledImageUpdateAfterBind = vk::Bool32{ true },
                .descriptorBindingStorageImageUpdateAfterBind = vk::Bool32{ true },
                .descriptorBindingStorageBufferUpdateAfterBind = vk::Bool32{ true },
                .descriptorBindingUniformTexelBufferUpdateAfterBind = vk::Bool32{ true },
                .descriptorBindingStorageTexelBufferUpdateAfterBind = vk::Bool32{ true },
                .descriptorBindingUpdateUnusedWhilePending = vk::Bool32{ true },
                .descriptorBindingPartiallyBound = vk::Bool32{ true },
                .descriptorBindingVariableDescriptorCount = vk::Bool32{true},

                .scalarBlockLayout = vk::Bool32{true},
                .timelineSemaphore = vk::Bool32{ true }
            },
            vk::PhysicalDeviceVulkan13Features
            {
                .synchronization2 = vk::Bool32{ true},
                .dynamicRendering = vk::Bool32{ true},
                .maintenance4 = vk::Bool32{true}
            }
        };

        const auto deviceCreateInfo = vk::DeviceCreateInfo
        {
            .pNext = &features.get(),
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = nullptr
        };

        const auto device = adapter.createDevice(deviceCreateInfo).value;

        for (auto& queue : queues)
        {
            const auto queueInfo = vk::DeviceQueueInfo2
            {
                .queueFamilyIndex = queue.get().familyIndex,
                .queueIndex = queue.get().queueIndex
            };
            queue.get().queue = device.getQueue2(queueInfo);
        }

        return device;
    }

    

    vk::PhysicalDevice selectAdapter(vk::Instance instance, const Features& requestedFeatures)
    {
        return instance.enumeratePhysicalDevices().value.front();
    }
}



RenderThread::RenderThread()
{
	instance = this;

}

VulkanRenderInterface::~VulkanRenderInterface()
{
  
    
}

std::unique_ptr<CommandList> VulkanRenderInterface::acquireCommandListInternal(QueueType queueType,
	CommandListType commandListType)
{
    assert(std::this_thread::get_id() == renderThreadId_);

    const auto commandBuffer = renderThreadCommandPoolData_.perQueueType[queueType][currentFrame_ % maxDeferredFrames_].commandBuffers.front();

    const auto beginInfo = vk::CommandBufferBeginInfo
    {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    commandBuffer.begin(beginInfo);

    return std::make_unique<VulkanCommandList>(commandBuffer, vk::CommandBufferLevel::ePrimary, queueType);
}

void VulkanRenderInterface::initializeInternal(const RendererDescriptor& descriptor)
{
    auto extensions = std::vector<const char*>{};
    
    auto layers = std::vector<const char*>{};
#ifdef TOY_ENGINE_ENABLE_VULKAN_VALIDATION
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

#ifdef TOY_ENGINE_VULKAN_BACKEND
    extensions.insert(extensions.end(),
        descriptor.meta.requiredExtensions.begin(),
        descriptor.meta.requiredExtensions.end());
#endif

    const auto applicationInfo = vk::ApplicationInfo()
        .setPApplicationName(descriptor.instanceName.c_str())
        .setApplicationVersion(descriptor.version)
        .setPEngineName(descriptor.instanceName.c_str())
        .setEngineVersion(descriptor.version)
        .setApiVersion(VK_API_VERSION_1_3);

    const auto instanceInfo = vk::InstanceCreateInfo()
        .setFlags(vk::InstanceCreateFlags())
        .setPApplicationInfo(&applicationInfo)
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPpEnabledExtensionNames(extensions.data())
        .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
        .setPpEnabledLayerNames(layers.data());

    vk::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);


    auto isSupported = vk::Bool32{};
    auto profileProperties = VpProfileProperties
	{
        .profileName = VP_LUNARG_DESKTOP_PORTABILITY_2021_NAME,
        .specVersion = VP_LUNARG_DESKTOP_PORTABILITY_2021_SPEC_VERSION
	};
    vpGetInstanceProfileSupport(nullptr, &profileProperties, &isSupported);

    assert(isSupported);

    const auto instanceCreateInfo = VpInstanceCreateInfo
    {
        .pCreateInfo = reinterpret_cast<const VkInstanceCreateInfo*>(&instanceInfo),
        .pProfile = &profileProperties
    };
    
    vpCreateInstance(&instanceCreateInfo, nullptr, reinterpret_cast<VkInstance*>(&instance_));

    /*instance_ = vk::createInstance(instanceInfo).value;*/
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);

    const auto requestedFeatures = Features{};
    adapter_ = selectAdapter(instance_, requestedFeatures);

    queues_[QueueType::graphics].familyIndex = getQueueFamilyIndex(adapter_, vk::QueueFlagBits::eGraphics);
    queues_[QueueType::transfer].familyIndex = getQueueFamilyIndex(adapter_, vk::QueueFlagBits::eTransfer);
    queues_[QueueType::asyncCompute].familyIndex = queues_[QueueType::graphics].familyIndex;

    device_ = createDevice(adapter_, { queues_[QueueType::graphics], queues_[QueueType::transfer], queues_[QueueType::asyncCompute] }, requestedFeatures);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device_);

#ifdef TOY_ENGINE_ENABLE_VULKAN_VALIDATION
    const auto debugUtilsMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT
    {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr
    };

    const auto debugUtilsHandler = instance_.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfo);
#endif

    const auto allocatorCreateInfo = VmaAllocatorCreateInfo
    {
        .physicalDevice = adapter_,
        .device = device_,
        .instance = instance_,
        .vulkanApiVersion = VK_API_VERSION_1_3
    };

    vmaCreateAllocator(&allocatorCreateInfo, &allocator_);

    //TODO: all create functions should report errors into a logger

    const auto surfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR
    {
        .hinstance = descriptor.handler.hinstance,
        .hwnd = descriptor.handler.hwnd
    };
    surface_ = instance_.createWin32SurfaceKHR(surfaceCreateInfo).value;

    const auto surfaceCapabilities = adapter_.getSurfaceCapabilitiesKHR(surface_).value;
    auto supportedCompositeAlpha =
        surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque
        ? vk::CompositeAlphaFlagBitsKHR::eOpaque
        : surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : vk::CompositeAlphaFlagBitsKHR::eInherit;

    auto formats = adapter_.getSurfaceFormatsKHR(surface_).value;
    const auto supportedFormat = formats.front();

    const auto extent = descriptor.windowExtentGetter();

    const auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR
    {
        .surface = surface_,
        .minImageCount = swapchainImagesCount_,
        .imageFormat = supportedFormat.format,
        .imageColorSpace = supportedFormat.colorSpace,
        .imageExtent = vk::Extent2D{ extent.width, extent.height },
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &queues_[QueueType::graphics].familyIndex,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = supportedCompositeAlpha,
        .presentMode = vk::PresentModeKHR::eFifo,
        .clipped = vk::Bool32{true},
        .oldSwapchain = nullptr
    };

    swapchain_ = device_.createSwapchainKHR(swapchainCreateInfo).value;

    swapchainImageAfterPresentFences_.resize(swapchainImagesCount_);
    swapchainImageViews_.resize(swapchainImagesCount_);

    swapchainImages_ = device_.getSwapchainImagesKHR(swapchain_).value;
    for (u32 i{}; i < swapchainImagesCount_; i++)
    {
        const auto imageViewCreateInfo = vk::ImageViewCreateInfo
        {
        	.image = swapchainImages_[i],
            .viewType = vk::ImageViewType::e2D,
            .format = supportedFormat.format,
            .subresourceRange = vk::ImageSubresourceRange
            {
                vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
            }
        };
    	swapchainImageViews_[i] = device_.createImageView(imageViewCreateInfo).value;

        swapchainImageAfterPresentFences_[i] = device_.createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled}).value;
    }

    
    readyToPresentSemaphore_ = device_.createSemaphore(vk::SemaphoreCreateInfo{}).value;

    readyToRenderSemaphore_ = device_.createSemaphore(vk::SemaphoreCreateInfo{}).value;


    renderThreadId_ = std::this_thread::get_id();

	renderThreadCommandPoolData_ = createPerThreadCommandPoolData(device_, vk::CommandBufferLevel::ePrimary, maxDeferredFrames_,
        queues_[QueueType::graphics].familyIndex, queues_[QueueType::asyncCompute].familyIndex, queues_[QueueType::transfer].familyIndex, 1, 1, 1);

}


void VulkanRenderInterface::deinitializeInternal()
{
    device_.waitIdle();


    for(const auto& [queueType, perFramePools]: renderThreadCommandPoolData_.perQueueType)
    {
	    for(const auto& perFrame: perFramePools)
	    {
            device_.destroyCommandPool(perFrame.commandPool);
	    }
    }

    device_.destroySemaphore(readyToPresentSemaphore_);
    device_.destroySemaphore(readyToRenderSemaphore_);

    for (auto i = u32{}; i < maxDeferredFrames_; i++)
    {
        device_.destroyImageView(swapchainImageViews_[i]);
        device_.destroyFence(swapchainImageAfterPresentFences_[i]);
    }

    //TODO: destroy swapchain image views
    device_.destroySwapchainKHR(swapchain_);
    instance_.destroy(surface_);
    for (u32 i{}; i < timelineSemaphorePerFrame_.size(); i++)
    {
        device_.destroySemaphore(timelineSemaphorePerFrame_[i]);
    }

    bufferPool_.clear();
    vmaDestroyAllocator(allocator_);
    device_.destroy();
}

void VulkanRenderInterface::nextFrameInternal()
{
    currentFrame_++;

    const auto nextFramesFence = swapchainImageAfterPresentFences_[currentFrame_ % maxDeferredFrames_];
    device_.waitForFences(1, &nextFramesFence, vk::Bool32{ true }, ~0ull);


    const auto pool = renderThreadCommandPoolData_.perQueueType[QueueType::graphics][currentFrame_ % maxDeferredFrames_].commandPool;
    device_.resetCommandPool(pool);
}

//Handle<RenderTarget> VulkanRenderInterface::createRenderTarget(RenderTargetDescriptor)
//{
//	return {};
//}
//
//Handle<Pipeline> VulkanRenderInterface::createPipeline(
//	const GraphicsPipelineDescriptor& graphicsPipelineDescription, const std::vector<BindGroupDescriptor>& bindGroups)
//{
//	return {};
//}

BindGroupLayout VulkanRenderInterface::allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor)
{
    auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
    auto bindingFlags = std::vector<vk::DescriptorBindingFlags>{};
    bindings.resize(descriptor.bindings.size());
    bindingFlags.resize(descriptor.bindings.size());
    for(u32 i{}; i < bindings.size(); i++)
    {
        bindings[i].binding = descriptor.bindings[i].binding;
        bindingFlags[i] = vk::DescriptorBindingFlagBits::eUpdateAfterBind;
        if(std::holds_alternative<SimpleDeclaration>(descriptor.bindings[i].descriptor))
        {
            const auto simpleBinding = std::get<SimpleDeclaration>(descriptor.bindings[i].descriptor);
            bindings[i].descriptorType = mapDescriptorType(simpleBinding.type);
            bindings[i].descriptorCount = 1;
        }
        if (std::holds_alternative<ArrayDeclaration>(descriptor.bindings[i].descriptor))
        {
            const auto arrayBinding = std::get<ArrayDeclaration>(descriptor.bindings[i].descriptor);
            bindings[i].descriptorType = mapDescriptorType(arrayBinding.type);
            bindings[i].descriptorCount = arrayBinding.elementsCount;
        }
        if (std::holds_alternative<BindlessDeclaration>(descriptor.bindings[i].descriptor))
        {
            //TODO: this binding should be the last one in the descriptor set
            const auto bindlessBinding = std::get<BindlessDeclaration>(descriptor.bindings[i].descriptor);
            bindings[i].descriptorType = mapDescriptorType(bindlessBinding.type);
            bindings[i].descriptorCount = bindlessBinding.maxDescriptorCount;
            bindingFlags[i] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
        }
        
    }

    //TODO: check for bindless feature

    const auto createInfo = vk::StructureChain
    {
        vk::DescriptorSetLayoutCreateInfo
        {
            .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
            .bindingCount = static_cast<u32>(bindings.size()),
            .pBindings = bindings.data()
        },
        vk::DescriptorSetLayoutBindingFlagsCreateInfo
        {
            .bindingCount = static_cast<u32>(bindings.size()),
            .pBindingFlags = bindingFlags.data()
        }
    };



    auto layout = device_.createDescriptorSetLayout(createInfo.get());

    return {};
}

SwapchainImage VulkanRenderInterface::acquireNextSwapchainImageInternal()
{
    const auto acquireInfo = vk::AcquireNextImageInfoKHR
    {
        .swapchain = swapchain_,
        .timeout = ~0u,
        .semaphore = readyToRenderSemaphore_,
        .fence = VK_NULL_HANDLE,
        .deviceMask = 1u
    };

    const auto nextImage = device_.acquireNextImage2KHR(acquireInfo).value;

    const auto imageView = swapchainImageViews_[nextImage];
    const auto image = swapchainImages_[nextImage];

    currentImageIndex_ = nextImage;

    return SwapchainImage
    {
        .image = std::make_unique<VulkanImage>(VulkanImage{.image = image }),
        .view = std::make_unique<VulkanImageView>(VulkanImageView{.vulkanImageView = imageView })
    };
}

void VulkanRenderInterface::presentInternal()
{
    auto imageIndices = std::array{ currentImageIndex_ };
    auto results = std::array{ vk::Result{} };

    const auto presentInfo = vk::PresentInfoKHR
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &readyToPresentSemaphore_,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_,
        .pImageIndices = imageIndices.data(),
        .pResults = results.data()
    };

    //TODO: should be a present queue
    const auto result = queues_[QueueType::graphics].queue.presentKHR(presentInfo);
    assert(result == vk::Result::eSuccess);

    device_.waitIdle();
}

void VulkanRenderInterface::submitCommandListInternal(const std::unique_ptr<CommandList> commandList)
{
    const auto& vulkanCommandList = dynamic_cast<VulkanCommandList&>(*commandList);

    vulkanCommandList.cmd_.end();

    const auto queue = queues_[vulkanCommandList.ownedQueueType_].queue;


    const auto commandBuffers = std::array
	{
        vk::CommandBufferSubmitInfo
        {
            .commandBuffer = vulkanCommandList.cmd_,
            .deviceMask = 1
        }

    };

    const auto signalSemaphoreSubmitInfo = vk::SemaphoreSubmitInfo
    {
        .semaphore = readyToPresentSemaphore_,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };

    const auto waitSemaphoreSubmitInfo = vk::SemaphoreSubmitInfo
    {
        .semaphore = readyToRenderSemaphore_,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };

    const auto submitInfo = vk::SubmitInfo2
    {
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo,
        .commandBufferInfoCount = static_cast<u32>(commandBuffers.size()),
        .pCommandBufferInfos = commandBuffers.data(),
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo

    };
    const auto fence = swapchainImageAfterPresentFences_[currentFrame_ % maxDeferredFrames_];
    device_.resetFences(1, &fence);
    const auto result = queue.submit2(1, &submitInfo, fence);
    //assert(result == vk::Result::eSuccess);
}
