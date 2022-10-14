#include "VulkanRenderInterface.h"

#include <iostream>
#include <map>
#include <vector>
#include <VulkanCommandList.h>
#include <Windows.h>
#include <vulkan/vulkan_profiles.hpp>
#include <fstream>

#include "Hash.h"
#include "g3log/g3log.hpp"
const LEVELS VULKAN_VALIDATION_ERROR{ WARNING.value + 1, {"VULKAN_VALIDATION_ERROR_LEVEL"} };
using namespace toy::renderer;
using namespace api::vulkan;
namespace
{
#define MAP_FLAG_BIT(srcFlag, srcFlagBit, dstFlag, dstFlagBit) if (srcFlag.containBit(srcFlagBit)) { dstFlag |= dstFlagBit; }

    vk::BufferUsageFlags vulkanMapAccessUsageFlag(const Flags<AccessUsage>& usage)
    {
        auto vulkanUsage = vk::BufferUsageFlags{};

        if(usage.containBit(AccessUsage::accelerationStructure))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
        }
        if (usage.containBit(AccessUsage::vertex))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
        }
        if (usage.containBit(AccessUsage::index))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
        }
        if (usage.containBit(AccessUsage::indirect))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eIndirectBuffer;
        }
        if (usage.containBit(AccessUsage::uniform))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
        }
        if (usage.containBit(AccessUsage::storage))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
        }
        if (usage.containBit(AccessUsage::transferDst))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eTransferDst;
        }
        if (usage.containBit(AccessUsage::transferSrc))
        {
            vulkanUsage |= vk::BufferUsageFlagBits::eTransferSrc;
        }

        return vulkanUsage;
    }

    vk::ShaderModule loadShader(const vk::Device device, const std::string& path)
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

    vk::Format mapFormat(Format format)
    {
        return vk::Format::eB8G8R8A8Unorm;
    }

    vk::DescriptorType mapDescriptorType(const BindingType type)
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


    vk::DescriptorSetLayout createGroupLayout(const vk::Device device, const BindGroupDescriptor& descriptor)
    {
        auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
        auto bindingFlags = std::vector<vk::DescriptorBindingFlags>{};
        bindings.resize(descriptor.bindings.size());
        bindingFlags.resize(descriptor.bindings.size());
        for (u32 i{}; i < bindings.size(); i++)
        {
            bindings[i].binding = descriptor.bindings[i].binding;
            bindingFlags[i] = vk::DescriptorBindingFlagBits::eUpdateAfterBind;
            bindings[i].stageFlags = vk::ShaderStageFlagBits::eAll; //TODO:: it's wrong
            if (std::holds_alternative<SimpleDeclaration>(descriptor.bindings[i].descriptor))
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



        return device.createDescriptorSetLayout(createInfo.get()).value;
    }



}



RenderThread::RenderThread()
{
	instance = this;
}

VulkanRenderInterface::~VulkanRenderInterface()
= default;

std::unique_ptr<CommandList> VulkanRenderInterface::acquireCommandListInternal(QueueType queueType,
	CommandListType commandListType)
{
    TOY_ASSERT(std::this_thread::get_id() == renderThreadId_);

    const auto commandBuffer = renderThreadCommandPoolData_.perQueueType[queueType][currentFrame_ % maxDeferredFrames_].commandBuffers.front();

    const auto beginInfo = vk::CommandBufferBeginInfo
    {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    const auto result = commandBuffer.begin(beginInfo);
    TOY_ASSERT(result == vk::Result::eSuccess);

    return std::make_unique<VulkanCommandList>(*this, commandBuffer, vk::CommandBufferLevel::ePrimary, queueType);
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

    TOY_ASSERT(isSupported);

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

    const auto& surfaceCapabilities = adapter_.getSurfaceCapabilitiesKHR(surface_).value;
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
        .presentMode = vk::PresentModeKHR::eFifo,//TODO: this limits to 60fps
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

    graphicsPipelineCache_.initialize(PipelineCacheDescriptor{ device_, 1024 });
    computePipelineCache_.initialize(PipelineCacheDescriptor{ device_, 1024 });

}


void VulkanRenderInterface::deinitializeInternal()
{
    const auto result = device_.waitIdle();
    TOY_ASSERT(result == vk::Result::eSuccess);

    //TODO: resource cleanup should move to resource manager
    {
        for(const auto& [key, buffer] : bufferStorage_)
        {
            //TODO: This should be fixed, not every buffer is host accessible and in a mapped state

            if(buffer.isMapped)
            {
                vmaUnmapMemory(allocator_, buffer.allocation);
            }

            vmaFreeMemory(allocator_, buffer.allocation);
            device_.destroyBuffer(buffer.buffer);
        }
        bufferStorage_.reset();

        for (const auto& [key, pipeline] : pipelineStorage_)
        {
            device_.destroyPipelineLayout(pipeline.layout);
            device_.destroyPipeline(pipeline.pipeline);
        }
        pipelineStorage_.reset();

        for (const auto& [key, shaderModule] : shaderModuleStorage_)
        {
            device_.destroyShaderModule(shaderModule.module);

        }
        shaderModuleStorage_.reset();

        graphicsPipelineCache_.deinitialize();
        computePipelineCache_.deinitialize();

    }

    for(const auto& [key, descriptorSetLayout] : bindGroupLayoutCache_)
    {
        device_.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    
    for(const auto& pools : descriptorPoolsPerFrame_)
    {
	    for(const auto& pool: pools)
	    {
            device_.resetDescriptorPool(pool);
            device_.destroyDescriptorPool(pool);
	    }
    }

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

    device_.destroySwapchainKHR(swapchain_);
    instance_.destroy(surface_);
    for (u32 i{}; i < timelineSemaphorePerFrame_.size(); i++)
    {
        device_.destroySemaphore(timelineSemaphorePerFrame_[i]);
    }

    vmaDestroyAllocator(allocator_);
    device_.destroy();
}

void VulkanRenderInterface::nextFrameInternal()
{
    bindGroupCache_.nextFrame();
    bindGroupStorage_.reset();
    currentFrame_++;

    const auto& nextFramesFence = swapchainImageAfterPresentFences_[currentFrame_ % maxDeferredFrames_];
    auto result = device_.waitForFences(1, &nextFramesFence, vk::Bool32{ true }, ~0ull);
    TOY_ASSERT(result == vk::Result::eSuccess);

    resetDescriptorPoolsUntilFrame((currentFrame_+maxDeferredFrames_ - 2)%maxDeferredFrames_);


    const auto pool = renderThreadCommandPoolData_.perQueueType[QueueType::graphics][currentFrame_ % maxDeferredFrames_].commandPool;
    result = device_.resetCommandPool(pool);
    TOY_ASSERT(result == vk::Result::eSuccess);
}

Handle<BindGroupLayout> VulkanRenderInterface::allocateBindGroupLayoutInternal(const BindGroupDescriptor& descriptor)
{
    const auto hash = Hasher::hash32(descriptor);
    if (bindGroupLayoutCache_.contains(hash))
    {
        return Handle<BindGroupLayout>{hash};
    }
    bindGroupLayoutCache_[hash] = createGroupLayout(device_, descriptor);

    return Handle<BindGroupLayout>{hash};
}

Handle<BindGroup> VulkanRenderInterface::allocateBindGroupInternal(
	const Handle<BindGroupLayout>& bindGroupLayout)
{
    return allocateBindGroupInternal(bindGroupLayout, 1).front();
}

std::vector<Handle<BindGroup>> VulkanRenderInterface::
allocateBindGroupInternal(const Handle<BindGroupLayout>& bindGroupLayout,
	const u32 bindGroupCount)
{
    const auto currentPools = currentFrame_ % swapchainImagesCount_;

    auto bindGroups = std::vector<Handle<BindGroup>>{};
    bindGroups.resize(bindGroupCount);
    auto poolIndex = u32{};
    auto cantAllocateDescriptorSet = true;
    while (cantAllocateDescriptorSet)
    {
        if (poolIndex >= descriptorPoolsPerFrame_[currentPools].size())
        {
            const auto poolSizes = std::array
            {
                vk::DescriptorPoolSize
                {
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = 1
                }
            };

            const auto poolCreateInfo = vk::DescriptorPoolCreateInfo
            {
                .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
                .maxSets = 1000,
                .poolSizeCount = poolSizes.size(),
                .pPoolSizes = poolSizes.data()
            };
            const auto result = device_.createDescriptorPool(poolCreateInfo);
            TOY_ASSERT(result.result == vk::Result::eSuccess);

            descriptorPoolsPerFrame_[currentPools].push_back(result.value);
        }
        else
        {
            const auto pool = descriptorPoolsPerFrame_[currentPools][poolIndex];

            auto setLayouts = std::vector<vk::DescriptorSetLayout>{};
            setLayouts.resize(bindGroupCount);
            std::fill(setLayouts.begin(), setLayouts.end(), bindGroupLayoutCache_[bindGroupLayout.index]);

            const auto allocateInfo = vk::DescriptorSetAllocateInfo
            {
                .descriptorPool = pool,
                .descriptorSetCount = bindGroupCount,
                .pSetLayouts = setLayouts.data()
            };
            const auto result = device_.allocateDescriptorSets(allocateInfo);

            switch (result.result)
            {
            case vk::Result::eSuccess:
                for(auto i = u32{}; i < bindGroupCount; i++)
                {
                    
                    bindGroups[i] = bindGroupStorage_.add(
                        VulkanBindGroup
                        {
	                    .descriptorSet = result.value[i]
                    });
                }
                
                cantAllocateDescriptorSet = false;
                break;
            case vk::Result::eErrorOutOfPoolMemory:
                poolIndex++;
                break;
            default:
                TOY_ASSERT(false); //TODO: fatal error
            }
        }
    }
    return bindGroups;
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

    const auto& imageView = swapchainImageViews_[nextImage];
    const auto& image = swapchainImages_[nextImage];

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
    TOY_ASSERT(result == vk::Result::eSuccess);
}

void VulkanRenderInterface::submitCommandListInternal(const std::unique_ptr<CommandList> commandList)
{
    const auto& vulkanCommandList = dynamic_cast<VulkanCommandList&>(*commandList);

    auto result = vulkanCommandList.cmd_.end();
    TOY_ASSERT(result == vk::Result::eSuccess);

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
    const auto& fence = swapchainImageAfterPresentFences_[currentFrame_ % maxDeferredFrames_];
    result = device_.resetFences(1, &fence);
    TOY_ASSERT(result == vk::Result::eSuccess);
    result = queue.submit2(1, &submitInfo, fence);
    TOY_ASSERT(result == vk::Result::eSuccess);
}

Handle<Pipeline> VulkanRenderInterface::createPipelineInternal(
	const GraphicsPipelineDescriptor& descriptor, const std::vector<SetBindGroupMapping>& bindGroups)
{

    const auto colorRenderTargets = static_cast<u32>(descriptor.renderTargetDescriptor.colorRenderTargets.size());
    auto colorRenderTargetFormats = std::vector<vk::Format>{};
    colorRenderTargetFormats.resize(colorRenderTargets);
    for (auto i = u32{}; i < colorRenderTargets; i++)
    {
        colorRenderTargetFormats[i] = mapFormat(descriptor.renderTargetDescriptor.colorRenderTargets[i].format);
    }

    const auto hasDepthRenderTarget = descriptor.renderTargetDescriptor.depthRenderTarget.has_value();
    const auto hasStencilRenderTarget = descriptor.renderTargetDescriptor.stencilRenderTarget.has_value();

    const auto stages = std::array
    {
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = shaderModuleStorage_.get(descriptor.vertexShader).module,
            .pName = "main"
        },
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = shaderModuleStorage_.get(descriptor.fragmentShader).module,
            .pName = "main"
        }
    };

    const auto setCount = static_cast<u32>(bindGroups.size());

    auto setLayouts = std::vector<vk::DescriptorSetLayout>{};
    setLayouts.resize(setCount);

    for (auto i = u32{}; i < setCount; i++)
    {
        setLayouts[i] = bindGroupLayoutCache_[bindGroups[i].bindGroupLayout.index];
    }

    const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo
    {
        .setLayoutCount = setCount,
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = 0
    };

    const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo{};
    const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo
    {
        .topology = vk::PrimitiveTopology::eTriangleList
    };
    auto viewportState = vk::PipelineViewportStateCreateInfo
    {
        .viewportCount = 1,
        .scissorCount = 1,
    };
    const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo
    {
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .lineWidth = 1.0f
    };
    auto multisampleState = vk::PipelineMultisampleStateCreateInfo
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1
    };
    auto colorAttachments = std::array
    {
        vk::PipelineColorBlendAttachmentState
        {
            .colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
        }
    };

    auto colorBlendState = vk::PipelineColorBlendStateCreateInfo
    {
        .attachmentCount = static_cast<uint32_t>(colorAttachments.size()),
        .pAttachments = colorAttachments.data()
    };
    auto dynamicStates = std::array
    {
        vk::DynamicState::eScissor,
        vk::DynamicState::eViewport
    };
    auto dynamicState = vk::PipelineDynamicStateCreateInfo
    {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };


    const auto pipelineLayoutResult = device_.createPipelineLayout(layoutCreateInfo);
    TOY_ASSERT(pipelineLayoutResult.result == vk::Result::eSuccess);

    const auto pipelinesInfo =
        vk::StructureChain
    {
        vk::GraphicsPipelineCreateInfo
        {
            .stageCount = static_cast<uint32_t>(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisampleState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayoutResult.value,
        },
        vk::PipelineRenderingCreateInfo
        {
            .viewMask = 0,
            .colorAttachmentCount = colorRenderTargets,
            .pColorAttachmentFormats = colorRenderTargetFormats.data(),
            .depthAttachmentFormat = hasDepthRenderTarget ? mapFormat(descriptor.renderTargetDescriptor.depthRenderTarget.value().format) : vk::Format::eUndefined,
            .stencilAttachmentFormat = hasStencilRenderTarget ? mapFormat(descriptor.renderTargetDescriptor.stencilRenderTarget.value().format) : vk::Format::eUndefined
        }
    };

    const auto pipelineResult = device_.createGraphicsPipeline(
        graphicsPipelineCache_.get(),
        pipelinesInfo.get());
    TOY_ASSERT(pipelineResult.result == vk::Result::eSuccess);

    const auto pipeline = VulkanPipeline{ .pipeline = pipelineResult.value, .layout = pipelineLayoutResult.value, .bindPoint = vk::PipelineBindPoint::eGraphics };
    return pipelineStorage_.add(pipeline, descriptor);
}

Handle<ShaderModule> VulkanRenderInterface::createShaderModuleInternal(ShaderStage stage,
	const ShaderCode& code)
{
    const auto moduleCreateInfo = vk::ShaderModuleCreateInfo
    {
        .codeSize = code.code.size()*4,
        .pCode = code.code.data()
    };
    const auto result = device_.createShaderModule(moduleCreateInfo);


    LOG_IF(FATAL, result.result != vk::Result::eSuccess) << "Shader module creation failed!";

    const auto shaderModule = VulkanShaderModule
	{
		.module = result.value
	};
    return shaderModuleStorage_.add(shaderModule);
}

void VulkanRenderInterface::resetDescriptorPoolsUntilFrame(const u32 frame)
{
    for(auto i = currentFrame_; i< frame; i++)
    {
        const auto& pools = descriptorPoolsPerFrame_[i % maxDeferredFrames_];

        for(const auto& pool: pools)
        {
            device_.resetDescriptorPool(pool);
            //TODO: performance optimization: decide how many pools should be deleted for the next frame
        }
    }
}

Handle<Buffer> VulkanRenderInterface::createBufferInternal(
	const BufferDescriptor& descriptor)
{
    
    const auto usage = vulkanMapAccessUsageFlag(descriptor.accessUsage);

    auto queues = std::vector<u32>{};
    queues.reserve(3);

    if(descriptor.queuesSharing.containBit(QueuesSharing::graphics))
    {
        queues.push_back(queues_[QueueType::graphics].familyIndex);
    }
    if (descriptor.queuesSharing.containBit(QueuesSharing::asyncCompute))
    {
        queues.push_back(queues_[QueueType::asyncCompute].familyIndex);
    }
    if (descriptor.queuesSharing.containBit(QueuesSharing::transfer))
    {
        queues.push_back(queues_[QueueType::transfer].familyIndex);
    }

    auto bufferCreateInfo = vk::BufferCreateInfo
    {
        .size = descriptor.size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    if(queues.size() > 1)
    {
        bufferCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
        bufferCreateInfo.queueFamilyIndexCount = static_cast<u32>(queues.size());
        bufferCreateInfo.pQueueFamilyIndices = queues.data();
    }


    //TODO: consider to use allocated pools
    const auto allocationCreateInfo = VmaAllocationCreateInfo
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, //TODO: it should depend on access pattern
        .usage = descriptor.memoryUsage == MemoryUsage::gpuOnly? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE : descriptor.memoryUsage == MemoryUsage::cpuOnly ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO
    };

    auto buffer = vk::Buffer{};
    auto allocation = VmaAllocation{};


    const auto result = static_cast<vk::Result>(vmaCreateBuffer(allocator_,
	    reinterpret_cast<VkBufferCreateInfo*>(&
		    bufferCreateInfo),
	    &allocationCreateInfo,
	    reinterpret_cast<VkBuffer*>(&buffer),
	    &allocation,
	    nullptr));

    TOY_ASSERT(result == vk::Result::eSuccess);

    const auto handle = bufferStorage_.add(VulkanBuffer{ buffer, allocation }, descriptor);

	return handle;
}

void VulkanRenderInterface::updateBindGroupInternal(
	const Handle<BindGroup>& bindGroup,
	const std::initializer_list<BindingDataMapping>& mappings)
{
    auto vulkanBindGroup = bindGroupStorage_.get(bindGroup);

    const auto mappingsVector = std::vector<BindingDataMapping>{ mappings };


    auto descriptorWrites = std::vector<vk::WriteDescriptorSet>{};
    descriptorWrites.reserve(mappings.size());

    auto descriptorInfos = std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>>{};
    descriptorInfos.resize(mappingsVector.size());

    for(auto i = u32{}; i < mappingsVector.size(); i++)
    {
        const auto& binding = mappingsVector[i];
        TOY_ASSERT(std::holds_alternative<CBV>(binding.view));
        const auto bufferView = std::get<CBV>(binding.view);
        const auto& vulkanBuffer = bufferStorage_.get(bufferView.bufferView.buffer);

        const auto descriptorBufferInfo = vk::DescriptorBufferInfo
        {
            .buffer = vulkanBuffer.buffer,
            .offset = bufferView.bufferView.offset,
            .range = bufferView.bufferView.size
        };

        descriptorInfos[i] = descriptorBufferInfo;
    }

    for (auto i = u32{}; i < mappingsVector.size(); i++)
	{
        const auto& binding = mappingsVector[i];
        //TODO:: TEMP
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

    device_.updateDescriptorSets(static_cast<u32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

}

void VulkanRenderInterface::mapInternal(const Handle<Buffer>& buffer, void** data)
{
    auto& vulkanBuffer = bufferStorage_.get(buffer);
    vulkanBuffer.isMapped = true;
    vmaMapMemory(allocator_, vulkanBuffer.allocation, data);
}



Handle<Pipeline> VulkanRenderInterface::createPipelineInternal(
	const ComputePipelineDescriptor& descriptor,
	const std::vector<SetBindGroupMapping>& bindGroups)
{
    const auto stage = vk::PipelineShaderStageCreateInfo
    {
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = shaderModuleStorage_.get(descriptor.computeShader).module,
        .pName = "main"
    };

    const auto setCount = static_cast<u32>(bindGroups.size());

    auto setLayouts = std::vector<vk::DescriptorSetLayout>{};
    setLayouts.resize(setCount);

    for (auto i = u32{}; i < setCount; i++)
    {
        setLayouts[i] = bindGroupLayoutCache_[bindGroups[i].bindGroupLayout.index];
    }

    const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo
    {
        .setLayoutCount = setCount,
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = 0
    };

    const auto pipelineLayoutResult = device_.createPipelineLayout(layoutCreateInfo);
    TOY_ASSERT(pipelineLayoutResult.result == vk::Result::eSuccess);

    const auto pipelinesInfo = vk::ComputePipelineCreateInfo
    {
        .stage = stage,
        .layout = pipelineLayoutResult.value
    };

    const auto pipelineResult = device_.createComputePipeline(computePipelineCache_.get(), pipelinesInfo);
    TOY_ASSERT(pipelineResult.result == vk::Result::eSuccess);

    const auto pipeline = VulkanPipeline{ .pipeline = pipelineResult.value, .layout = pipelineLayoutResult.value, .bindPoint = vk::PipelineBindPoint::eCompute };
    return pipelineStorage_.add(pipeline, descriptor);
}
