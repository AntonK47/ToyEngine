#include "VulkanRenderInterface.h"

#include <Windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <vulkan/vulkan_win32.h>
#include <VulkanCommandList.h>

#define VULKAN_VALIDATION

using namespace toy::renderer;
using namespace api::vulkan;
namespace
{
#define MAP_FLAG_BIT(srcFlag, srcFlagBit, dstFlag, dstFlagBit) if (srcFlag.containBit(srcFlagBit)) { dstFlag |= dstFlagBit; }

    

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

                graphicsPerFrame[i].commandPool = device.createCommandPool(commandPoolCreateInfo);

                const auto allocateInfo = vk::CommandBufferAllocateInfo
                {
                    .commandPool = graphicsPerFrame[i].commandPool,
                    .level = level,
                    .commandBufferCount = graphicsCommandListPerFrame
                };
                graphicsPerFrame[i].commandBuffers = device.allocateCommandBuffers(allocateInfo);
	        }
            if (asyncComputeCommandListPerFrame != 0)
            {
                const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
                {
                    .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                    .queueFamilyIndex = asyncComputeIndex
                };

                asyncComputePerFrame[i].commandPool = device.createCommandPool(commandPoolCreateInfo);

                const auto allocateInfo = vk::CommandBufferAllocateInfo
                {
                    .commandPool = asyncComputePerFrame[i].commandPool,
                    .level = level,
                    .commandBufferCount = asyncComputeCommandListPerFrame
                };
                asyncComputePerFrame[i].commandBuffers = device.allocateCommandBuffers(allocateInfo);
            }
            if (transferCommandListPerFrame != 0)
            {
                const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
                {
                    .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                    .queueFamilyIndex = transferIndex
                };

                transferPerFrame[i].commandPool = device.createCommandPool(commandPoolCreateInfo);

                const auto allocateInfo = vk::CommandBufferAllocateInfo
                {
                    .commandPool = transferPerFrame[i].commandPool,
                    .level = level,
                    .commandBufferCount = transferCommandListPerFrame
                };
                transferPerFrame[i].commandBuffers = device.allocateCommandBuffers(allocateInfo);
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

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:  // NOLINT(bugprone-branch-clone)
                //std::cout << defaultColorSequence << "[Verbose]: " << pCallbackData->pMessage << std::endl;
            return vk::Bool32{ false };
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            //std::cout << infoColorSequence << "[Info]: " << pCallbackData->pMessage << std::endl;
            return vk::Bool32{ false };
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            std::cout << warningColorSequence << "[Warning]: " << pCallbackData->pMessage << std::endl;
            return vk::Bool32{ false };
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            std::cout << errorColorSequence << "[Error]: " << pCallbackData->pMessage << std::endl;
            break;
        }

        std::cout << defaultColorSequence;

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

        const auto device = adapter.createDevice(deviceCreateInfo);

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
        return instance.enumeratePhysicalDevices().front();
    }
}



RenderThread::RenderThread()
{
	instance = this;

}

VulkanRenderInterface::VulkanRenderInterface()
{
    /*
     *  1.external vulkan extensions should be abstract
     *      it some kind of interact with window system, like SDL2
     *  2.fix dynamic vulkan dispatcher
     *  3.make proper adapter selection
     *  4.make proper device creation
     *  5 error handling
     */
    

    /*const auto semaphoreCreateInfo = vk::StructureChain
    {
        vk::SemaphoreCreateInfo{},
        vk::SemaphoreTypeCreateInfo
        {
            .semaphoreType = vk::SemaphoreType::eTimeline,
            .initialValue = 0
        }
    };
    
    timelineSemaphorePerFrame_.resize(maxDeferredFrames_);
    for(u32 i{}; i < timelineSemaphorePerFrame_.size(); i++)
    {
	    timelineSemaphorePerFrame_[i] = device_.createSemaphore(semaphoreCreateInfo.get<vk::SemaphoreCreateInfo>());
    }*/
}

VulkanRenderInterface::~VulkanRenderInterface()
{
  
    for (u32 i{}; i < timelineSemaphorePerFrame_.size(); i++)
    {
        device_.destroySemaphore(timelineSemaphorePerFrame_[i]);
    }

    bufferPool_.clear();
    vmaDestroyAllocator(allocator_);
    device_.destroy();
}

std::unique_ptr<CommandList> VulkanRenderInterface::acquireCommandList(QueueType queueType,
	CommandListType commandListType)
{
    assert(std::this_thread::get_id() == renderThreadId_);

    const auto commandBuffer = renderThreadCommandPoolData_.perQueueType[queueType][currentFrame_ % maxDeferredFrames_].commandBuffers.front();
    return std::make_unique<VulkanCommandList>(commandBuffer, vk::CommandBufferLevel::ePrimary);
}

void VulkanRenderInterface::initialize(RendererDescriptor descriptor)
{
    auto extensions = std::vector<const char*>{};
    
    auto layers = std::vector<const char*>{};
#if defined(VULKAN_VALIDATION)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

#if defined(VULKAN_BACKEND)
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
    instance_ = vk::createInstance(instanceInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);

    const auto requestedFeatures = Features{};
    adapter_ = selectAdapter(instance_, requestedFeatures);

    queues_[QueueType::graphics].familyIndex = getQueueFamilyIndex(adapter_, vk::QueueFlagBits::eGraphics);
    queues_[QueueType::transfer].familyIndex = getQueueFamilyIndex(adapter_, vk::QueueFlagBits::eTransfer);
    queues_[QueueType::asyncCompute].familyIndex = queues_[QueueType::graphics].familyIndex;

    device_ = createDevice(adapter_, { queues_[QueueType::graphics], queues_[QueueType::transfer], queues_[QueueType::asyncCompute] }, requestedFeatures);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device_);

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
    surface_ = instance_.createWin32SurfaceKHR(surfaceCreateInfo);

    const auto surfaceCapabilities = adapter_.getSurfaceCapabilitiesKHR(surface_);
    auto supportedCompositeAlpha =
        surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque
        ? vk::CompositeAlphaFlagBitsKHR::eOpaque
        : surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : vk::CompositeAlphaFlagBitsKHR::eInherit;

    auto formats = adapter_.getSurfaceFormatsKHR(surface_);
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

    swapchain_ = device_.createSwapchainKHR(swapchainCreateInfo);

    swapchainImageViews_.resize(swapchainImagesCount_);

    auto swapchainImages = device_.getSwapchainImagesKHR(swapchain_);
    for (u32 i{}; i < swapchainImagesCount_; i++)
    {
        const auto imageViewCreateInfo = vk::ImageViewCreateInfo
        {
        	.image = swapchainImages[i],
            .viewType = vk::ImageViewType::e2D,
            .format = supportedFormat.format,
            .subresourceRange = vk::ImageSubresourceRange
            {
                vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
            }
        };
    	swapchainImageViews_[i] = device_.createImageView(imageViewCreateInfo);
    }



    renderThreadId_ = std::this_thread::get_id();

	renderThreadCommandPoolData_ = createPerThreadCommandPoolData(device_, vk::CommandBufferLevel::ePrimary, maxDeferredFrames_,
        queues_[QueueType::graphics].familyIndex, queues_[QueueType::asyncCompute].familyIndex, queues_[QueueType::transfer].familyIndex, 1, 1, 1);

}


void VulkanRenderInterface::deinitialize()
{
    device_.waitIdle();

    //TODO: destroy swapchain image views
    device_.destroySwapchainKHR(swapchain_);
    instance_.destroy(surface_);
}

void VulkanRenderInterface::nextFrame()
{
}

Handle<RenderTarget> VulkanRenderInterface::createRenderTarget(RenderTargetDescription)
{
	return {};
}

Handle<Pipeline> VulkanRenderInterface::createPipeline(
	const GraphicsPipelineDescription& graphicsPipelineDescription, const std::vector<BindGroup>& bindGroups)
{
	return {};
}
