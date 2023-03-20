#include "VulkanRenderInterface.h"

#include <iostream>
#include <g3log/g3log.hpp>

#include "VulkanMappings.h"

const LEVELS VULKAN_VALIDATION_ERROR{ WARNING.value + 1, {"VULKAN_VALIDATION_ERROR_LEVEL"} };
using namespace toy::renderer;
using namespace api::vulkan;

namespace
{
#define MAP_FLAG_BIT(srcFlag, srcFlagBit, dstFlag, dstFlagBit) if (srcFlag.containBit(srcFlagBit)) { dstFlag |= dstFlagBit; }



    template<typename T>
    uint64_t vkHppToHandler(T object)
    {
        return reinterpret_cast<uint64_t>(static_cast<typename T::CType>(object));
    }

    inline void setObjectName(const vk::Device device, const vk::ObjectType type, const uint64_t handle, const char* name) noexcept
    {
#ifdef _DEBUG
        const auto nameInfo = vk::DebugUtilsObjectNameInfoEXT
        {
            .objectType = type,
            .objectHandle = handle,
            .pObjectName = name
        };

        const auto result = device.setDebugUtilsObjectNameEXT(nameInfo);
        TOY_ASSERT(result == vk::Result::eSuccess);
#endif
    }

    template<class T>
    concept VulkanHppHandleType = vk::isVulkanHandleType<T>::value;

    ////TODO: ?
    //template<class T>
    //concept VulkanHandleType = std::is_same_v<T, VkDeviceMemory>;
    //
    //inline void setObjectName(const vk::Device device, VulkanHandleType auto handle, const char* name) noexcept
    //{
    //    setObjectName(device, decltype(handle), handle, name);
    //}

    template<VulkanHppHandleType T>
    inline void setObjectName(const vk::Device device, T handle, const char* name) noexcept
    {
        setObjectName(device, T::objectType, vkHppToHandler(handle), name);
    }

    template<typename T>
    inline void setObjectName(const vk::Device device, T handle, const std::string& name) noexcept
    {
        setObjectName(device, handle, name.c_str());
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

        //DebugBreak();

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

    vk::Device createDevice(const vk::PhysicalDevice adapter, const std::vector<std::reference_wrapper<DeviceQueue>>& queues, const Features& requiredFeatures)
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

        //TODO: Ray tracing and Acceleration structure extensions do not work properly with enabled render doc layer, it should be configured in cmake
        auto extensions = std::vector
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef TOY_ENGINE_ENABLE_RAY_TRACING
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME,
#endif
        };

        const auto features = vk::StructureChain
        {
            vk::PhysicalDeviceFeatures2
            {
                .features = vk::PhysicalDeviceFeatures
                {
                    .shaderInt64 = vk::Bool32{true},
                    .sparseBinding = vk::Bool32{true},
                    .sparseResidencyImage2D = vk::Bool32{true}
                }
            },
            vk::PhysicalDeviceVulkan11Features
            {
                .shaderDrawParameters = vk::Bool32{true}
            },
            vk::PhysicalDeviceVulkan12Features
            {
                .storageBuffer8BitAccess = vk::Bool32{ true},
            	.shaderBufferInt64Atomics = vk::Bool32{true},
                .descriptorIndexing = vk::Bool32{ true },

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
                .timelineSemaphore = vk::Bool32{ true },
                .bufferDeviceAddress = vk::Bool32{ true},

            },
            vk::PhysicalDeviceVulkan13Features
            {
                .synchronization2 = vk::Bool32{ true},
                .dynamicRendering = vk::Bool32{ true},
                .maintenance4 = vk::Bool32{true}
            },
            vk::PhysicalDeviceDescriptorBufferFeaturesEXT
            {
                .descriptorBuffer = vk::Bool32{true}
            },
#ifdef TOY_ENGINE_ENABLE_RAY_TRACING
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR
            {
                .accelerationStructure = vk::Bool32{true},
                .descriptorBindingAccelerationStructureUpdateAfterBind = vk::Bool32{true}
            },
            vk::PhysicalDeviceRayTracingPipelineFeaturesKHR
            {
                .rayTracingPipeline = vk::Bool32{true}
            },
            vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR
            {
                .rayTracingMaintenance1 = vk::Bool32{true}
            },
#endif
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

        const auto result = adapter.createDevice(deviceCreateInfo);

        TOY_ASSERT(result.result == vk::Result::eSuccess);

        const auto device = result.value;

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
    
    //TODO: if enable synitizer __declspec(no_sanitize_address)
    vk::CompositeAlphaFlagBitsKHR getSupportedCompositeAlphaFlag(vk::CompositeAlphaFlagsKHR flag)
    {
       
            const auto supportedCompositeAlpha =
                flag & vk::CompositeAlphaFlagBitsKHR::eOpaque
            ? vk::CompositeAlphaFlagBitsKHR::eOpaque
            : flag & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
            ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
            : flag & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
            : vk::CompositeAlphaFlagBitsKHR::eInherit;
            return supportedCompositeAlpha;
    }

    vk::PhysicalDevice selectAdapter(vk::Instance instance, const Features& requestedFeatures)
    {
        

        const auto& adapters = instance.enumeratePhysicalDevices().value;
        TOY_ASSERT(!adapters.empty());
        const auto& adapter = adapters.front();

        //const auto& properties = adapter.enumerateDeviceExtensionProperties().value;

        return adapter;
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

            {
                const auto bindingDescriptor = descriptor.bindings[i].descriptor;
                bindings[i].descriptorType = mapDescriptorType(bindingDescriptor.type);
                bindings[i].descriptorCount = bindingDescriptor.descriptorCount;

                const auto isLastBinding = i == bindings.size() - 1;
                if (descriptor.flags.containBit(BindGroupFlag::unboundLast) && isLastBinding)
                {
                    bindingFlags[i] |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
                }

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

        const auto result = device.createDescriptorSetLayout(createInfo.get());

        setObjectName(device, result.value, "DSL[" + std::to_string(bindings.size()) + "]");
        return result.value;
    }
}

//TODO: make it better
static int aa = 0;
std::mutex m;
VulkanRenderInterface::CommandListType VulkanRenderInterface::acquireCommandListInternal(QueueType queueType, const WorkerThreadId workerId, const UsageScope& usageScope)
{
    //TOY_ASSERT(std::this_thread::get_id() == renderThreadId_);

    //TODO: create proper structure to hold per thread data
    std::lock_guard lock(m);
    const auto& commandBuffer = perThreadData_[workerId.index].perQueueType[queueType][currentFrame_ % maxDeferredFrames_].commandBuffers[aa];
    aa++;

    return CommandListType(queueType, *this, commandBuffer);
}

void VulkanRenderInterface::submitBatchesInternal(const QueueType queueType,
    const std::initializer_list<SubmitBatchType>& batches)
{
    auto totalCommandBuffers = u32{};

    for(const auto& batch : batches)
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
	    for(auto i = u32{}; i < batch.batch_.commandBuffersCount; i++)
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

    if(minGraphicsValue!=0)
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
    auto result = queue.submit2(1, &submitInfo, nullptr);
    TOY_ASSERT(result == vk::Result::eSuccess);
    result = device_.waitIdle();
    TOY_ASSERT(result == vk::Result::eSuccess);
}

VulkanRenderInterface::SubmitBatchType VulkanRenderInterface::submitCommandListInternal(
    const QueueType queueType,
	const std::initializer_list<CommandListType>& commandLists,
	const std::initializer_list<SubmitDependency>& dependencies)
{
    TOY_ASSERT(!std::empty(commandLists));
    auto hasDependency = std::array{ false, false, false };
    auto maxValue = std::array{ u64{}, u64{}, u64{} };

    for(const auto& [queue, value] : dependencies)
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

    for(const auto& commandList : commandLists)
    {
        TOY_ASSERT(commandList.getQueueType() == queueType);
        submit.commandBuffers[submit.commandBuffersCount] = commandList.commandBuffer_;
        submit.commandBuffersCount++;
    }
    
    return VulkanSubmitBatch{ submit, queueType };
}


void VulkanRenderInterface::initializeInternal(const RendererDescriptor& descriptor)
{
    auto extensions = std::vector<const char*>{};

    auto layers = std::vector<const char*>{};
#ifdef TOY_ENGINE_ENABLE_VULKAN_VALIDATION
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
#ifdef TOY_ENGINE_ENABLE_RENDER_DOC_CAPTURING
    layers.push_back("VK_LAYER_RENDERDOC_Capture");
#endif

#ifdef TOY_ENGINE_VULKAN_BACKEND
    extensions.insert(extensions.end(),
        descriptor.meta.requiredExtensions.begin(),
        descriptor.meta.requiredExtensions.end());
#endif

    const auto& applicationInfo = vk::ApplicationInfo()
        .setPApplicationName(descriptor.instanceName.c_str())
        .setApplicationVersion(descriptor.version)
        .setPEngineName(descriptor.instanceName.c_str())
        .setEngineVersion(descriptor.version)
        .setApiVersion(VK_API_VERSION_1_3);

    const auto& instanceInfo = vk::InstanceCreateInfo()
        .setFlags(vk::InstanceCreateFlags())
        .setPApplicationInfo(&applicationInfo)
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPpEnabledExtensionNames(extensions.data())
        .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
        .setPpEnabledLayerNames(layers.data());

    vk::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    //TODO: find matching profile with ray tracing support
    /*auto isSupported = vk::Bool32{};
    auto profileProperties = VpProfileProperties
    {
        .profileName = VP_LUNARG_DESKTOP_PORTABILITY_2021_NAME,
        .specVersion = VP_LUNARG_DESKTOP_PORTABILITY_2021_SPEC_VERSION
    };
    vpGetInstanceProfileSupport(nullptr, &profileProperties, &isSupported);*/

    //TOY_ASSERT(isSupported);

    /*const auto instanceCreateInfo = VpInstanceCreateInfo
    {
        .pCreateInfo = reinterpret_cast<const VkInstanceCreateInfo*>(&instanceInfo),
        .pProfile = &profileProperties
    };

    vpCreateInstance(&instanceCreateInfo, nullptr, reinterpret_cast<VkInstance*>(&instance_));*/

    const auto result = vk::createInstance(instanceInfo);

    TOY_ASSERT(result.result == vk::Result::eSuccess);

    instance_ = result.value;

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
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
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
    const auto& formats = adapter_.getSurfaceFormatsKHR(surface_).value;
    const auto& supportedFormat = formats.front();

    {
        const auto& surfaceCapabilities = adapter_.getSurfaceCapabilitiesKHR(surface_).value;
        
        const auto supportedCompositeAlpha = getSupportedCompositeAlphaFlag(surfaceCapabilities.supportedCompositeAlpha);

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
            .presentMode = vk::PresentModeKHR::eMailbox,//vk::PresentModeKHR::eFifo,//TODO: this limits to 60fps
            .clipped = vk::Bool32{true},
            .oldSwapchain = nullptr
        };

        swapchain_ = device_.createSwapchainKHR(swapchainCreateInfo).value;
    }

    swapchainImageAfterPresentFences_.resize(swapchainImagesCount_);
    auto swapchainImageViews = std::vector<vk::ImageView>{};
    swapchainImageViews.resize(swapchainImagesCount_);

    const auto& swapchainImages = device_.getSwapchainImagesKHR(swapchain_).value;
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
        swapchainImageViews[i] = device_.createImageView(imageViewCreateInfo).value;

        swapchainImageAfterPresentFences_[i] = device_.createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }).value;
    }

    for (auto i = u32{}; i < swapchainImages.size(); i++)
    {
        swapchainImages_.push_back(imageStorage_.add(VulkanImage{ swapchainImages[i], {},false, true }));
        swapchainImageViews_.push_back(imageViewStorage_.add(VulkanImageView{ swapchainImageViews[i] }));
    }


    readyToPresentSemaphore_ = device_.createSemaphore(vk::SemaphoreCreateInfo{}).value;

    readyToRenderSemaphore_ = device_.createSemaphore(vk::SemaphoreCreateInfo{}).value;



    const auto timelineSemaphoreCreateInfo = vk::StructureChain
    {
        vk::SemaphoreCreateInfo{},
        vk::SemaphoreTypeCreateInfo
        {
            .semaphoreType = vk::SemaphoreType::eTimeline,
            .initialValue = u64{}
        }
    };

    timelineSemaphorePerQueue_[QueueType::graphics] = device_.createSemaphore(timelineSemaphoreCreateInfo.get()).value;
    timelineSemaphorePerQueue_[QueueType::asyncCompute] = device_.createSemaphore(timelineSemaphoreCreateInfo.get()).value;
    timelineSemaphorePerQueue_[QueueType::transfer] = device_.createSemaphore(timelineSemaphoreCreateInfo.get()).value;
    
    

    renderThreadId_ = std::this_thread::get_id();

    perThreadData_.resize(descriptor.threadWorkersCount);
    for (auto i = u32{}; i < descriptor.threadWorkersCount; i++)
    {
        perThreadData_[i] = initializePerRenderThreadData();
    }

	/*renderThreadCommandPoolData_ = createPerThreadCommandPoolData(device_, maxDeferredFrames_,
        queues_[QueueType::graphics].familyIndex, queues_[QueueType::asyncCompute].familyIndex, queues_[QueueType::transfer].familyIndex, 1, 1, 1);*/

    graphicsPipelineCache_.initialize(PipelineCacheDescriptor{ device_, 1024 });
    computePipelineCache_.initialize(PipelineCacheDescriptor{ device_, 1024 });

    nativeBackend_.device = device_;
    nativeBackend_.instance = instance_;

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

        for (const auto& [key, image] : imageStorage_)
        {
            //TODO: This should be fixed, not every buffer is host accessible and in a mapped state

            if (image.isMapped)
            {
                vmaUnmapMemory(allocator_, image.allocation);
            }
            if(!image.isExternal)
            {
	            vmaFreeMemory(allocator_, image.allocation);
				device_.destroyImage(image.image);
            }
        }
        imageStorage_.reset();

        for (const auto& [key, imageView] : imageViewStorage_)
        {
        	device_.destroyImageView(imageView.imageView);
        }
        imageViewStorage_.reset();

        for (const auto& [key, sampler] : samplerStorage_)
        {
            device_.destroySampler(sampler.sampler);
        }
        samplerStorage_.reset();

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
        device_.destroyDescriptorSetLayout(descriptorSetLayout.layout);
    }
    bindGroupLayoutCache_.clear();

    for(const auto& pools : descriptorPoolsPerFrame_)
    {
	    for(const auto& pool: pools)
	    {
            device_.resetDescriptorPool(pool);
            device_.destroyDescriptorPool(pool);
	    }
    }

    for (const auto& pool : descriptorPoolsPersistent_)
    {
        device_.resetDescriptorPool(pool);
        device_.destroyDescriptorPool(pool);
    }

    for (auto& threadData : perThreadData_)
    {
        for (const auto& [queueType, perFramePools] : threadData.perQueueType)
        {
            for (const auto& perFrame : perFramePools)
            {
                device_.destroyCommandPool(perFrame.commandPool);
            }
        }
    }
    

    device_.destroySemaphore(readyToPresentSemaphore_);
    device_.destroySemaphore(readyToRenderSemaphore_);

    for (auto i = u32{}; i < maxDeferredFrames_; i++)
    {
        device_.destroyFence(swapchainImageAfterPresentFences_[i]);
    }

    for(const auto& [key, timeline] : timelineSemaphorePerQueue_)
    {
        device_.destroySemaphore(timeline);
    }
    timelineSemaphorePerQueue_.clear();

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
    //TODO: hack
    aa = 0;
    const auto& nextFramesFence = swapchainImageAfterPresentFences_[(currentFrame_ + u64{ 1 }) % maxDeferredFrames_];
    auto result = device_.waitForFences(1, &nextFramesFence, vk::Bool32{ true }, ~0ull);
    TOY_ASSERT(result == vk::Result::eSuccess);

    //TODO: reset all thread pools at once
    for (auto& threadData : perThreadData_)
    {
        const auto pool = threadData.perQueueType[QueueType::graphics][currentFrame_ % maxDeferredFrames_].commandPool;
        result = device_.resetCommandPool(pool);
        TOY_ASSERT(result == vk::Result::eSuccess);
    }
    

    resetDescriptorPoolsUntilFrame(currentFrame_);
}

Handle<BindGroupLayout> VulkanRenderInterface::createBindGroupLayoutInternal(const BindGroupDescriptor& descriptor)
{
    const auto hash = Hasher::hash32(descriptor);
    if (bindGroupLayoutCache_.contains(hash))
    {
        return Handle<BindGroupLayout>{hash};
    }

    auto bindGroup = VulkanBindGroupLayout{};

	bindGroup.layout = createGroupLayout(device_, descriptor);
    if(descriptor.flags.containBit(BindGroupFlag::unboundLast))
    {
        bindGroup.lastBindVariableSize = descriptor.bindings.back().descriptor.descriptorCount;
    }

    bindGroupLayoutCache_[hash] = bindGroup;

    return Handle<BindGroupLayout>{hash};
}

std::vector<Handle<BindGroup>> VulkanRenderInterface::
allocateBindGroupInternal(const Handle<BindGroupLayout>& bindGroupLayout,
	const u32 bindGroupCount, const UsageScope& scope)
{
    const auto currentPoolsIndex = currentFrame_ % swapchainImagesCount_;
    auto& currentPools = 
        scope == UsageScope::async ?
        descriptorPoolsPersistent_ :
		descriptorPoolsPerFrame_[currentPoolsIndex];

    auto bindGroups = std::vector<Handle<BindGroup>>{};//TODO: smallvector
    bindGroups.resize(bindGroupCount);
    auto poolIndex = u32{};
    auto cantAllocateDescriptorSet = true;
    while (cantAllocateDescriptorSet)
    {
        if (poolIndex >= currentPools.size())
        {
            const auto poolSizes = std::array
            {
                //TODO: compute proper sizes for each descriptor type
                vk::DescriptorPoolSize
                {
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = 10
                },
                vk::DescriptorPoolSize
                {
                    .type = vk::DescriptorType::eStorageBuffer,
                    .descriptorCount = 10
                }
            };

            const auto poolCreateInfo = vk::DescriptorPoolCreateInfo
            {
                .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
                .maxSets = 100,
                .poolSizeCount = poolSizes.size(),
                .pPoolSizes = poolSizes.data()
            };
            const auto result = device_.createDescriptorPool(poolCreateInfo);
            TOY_ASSERT(result.result == vk::Result::eSuccess);

            currentPools.push_back(result.value);
        }
        else
        {
            const auto& pool = currentPools[poolIndex];

            auto setLayouts = std::vector<vk::DescriptorSetLayout>{};//TODO: smallvector
            setLayouts.resize(bindGroupCount);

            auto lastBindVariableSize = std::vector<u32>{};//TODO: smallvector
            lastBindVariableSize.resize(bindGroupCount);

            std::fill(setLayouts.begin(), setLayouts.end(), bindGroupLayoutCache_[bindGroupLayout.index].layout);

            std::fill(lastBindVariableSize.begin(), lastBindVariableSize.end(), bindGroupLayoutCache_[bindGroupLayout.index].lastBindVariableSize);

            const auto allocateInfo = vk::StructureChain
            {
                vk::DescriptorSetAllocateInfo
	            {
	                .descriptorPool = pool,
	                .descriptorSetCount = bindGroupCount,
	                .pSetLayouts = setLayouts.data()
	            },
                vk::DescriptorSetVariableDescriptorCountAllocateInfo
                {
                    .descriptorSetCount = static_cast<u32>(lastBindVariableSize.size()),
                    .pDescriptorCounts = lastBindVariableSize.data()
                }
            };

            auto descriptorSets = std::vector<vk::DescriptorSet>{};//TODO: smallvector
            descriptorSets.resize(bindGroupCount);
            
            const auto result = device_.allocateDescriptorSets(&allocateInfo.get(), descriptorSets.data());

            switch (result)
            {
            case vk::Result::eSuccess:
                for(auto i = u32{}; i < bindGroupCount; i++)
                {
#ifdef TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
                    setObjectName(device_, descriptorSets[i], "DS");
#endif
                    if(scope == UsageScope::async)
                    {
                        bindGroups[i] = persistentBindGroupStorage_.add(
                            VulkanBindGroup
                            {
                            .descriptorSet = descriptorSets[i]
                            }, BindGroup { scope });
                    }
                    else
                    {
                        bindGroups[i] = bindGroupStorage_.add(
                            VulkanBindGroup
                            {
                            .descriptorSet = descriptorSets[i]
                            }, BindGroup{ scope });
                    }
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

    const auto result = device_.acquireNextImage2KHR(acquireInfo);
    TOY_ASSERT(result.result == vk::Result::eSuccess);
    const auto nextImage = result.value;
    
    const auto& imageView = swapchainImageViews_[nextImage];
    const auto& image = swapchainImages_[nextImage];

    currentImageIndex_ = nextImage;

    return SwapchainImage
    {
        .image = image,
        .view = imageView
    };
}

void VulkanRenderInterface::presentInternal(const SubmitDependency& dependency)
{
    auto imageIndices = std::array{ currentImageIndex_ };
    auto results = std::array{ vk::Result{} };

    const auto signalSemaphoreSubmitInfo = vk::SemaphoreSubmitInfo
    {
        .semaphore = readyToPresentSemaphore_,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };

    auto waitInfos = std::vector<vk::SemaphoreSubmitInfo>{};//TODO: smallvector
    waitInfos.reserve(2);
    waitInfos.push_back
        (
            vk::SemaphoreSubmitInfo
            {
                .semaphore = timelineSemaphorePerQueue_[QueueType::graphics],
                .value = dependency.value,
                .stageMask = vk::PipelineStageFlagBits2::eAllCommands, //TODO: select something smarter
                .deviceIndex = 0
            }
        );

    const auto waitSemaphoreSubmitInfo = vk::SemaphoreSubmitInfo
    {
        .semaphore = readyToRenderSemaphore_,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };

    waitInfos.push_back(waitSemaphoreSubmitInfo);

    const auto submitInfo = vk::SubmitInfo2
    {
        .waitSemaphoreInfoCount = static_cast<u32>(std::size(waitInfos)),
        .pWaitSemaphoreInfos = std::data(waitInfos),
        .commandBufferInfoCount = 0,
        .pCommandBufferInfos = nullptr,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo
    };

    const auto& fence = swapchainImageAfterPresentFences_[currentFrame_ % maxDeferredFrames_];
    auto result = device_.resetFences(1, &fence);
    TOY_ASSERT(result == vk::Result::eSuccess);

    const auto queue = queues_[QueueType::graphics].queue;
    result = queue.submit2(1, &submitInfo, fence);
    TOY_ASSERT(result == vk::Result::eSuccess);

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
    result = queues_[QueueType::graphics].queue.presentKHR(presentInfo);
    TOY_ASSERT(result == vk::Result::eSuccess);
}

void VulkanRenderInterface::submitCommandListInternal(const CommandListType& commandList)
{
    const auto queue = queues_[commandList.getQueueType()].queue;

    const auto commandBuffers = std::array
	{
        vk::CommandBufferSubmitInfo
        {
            .commandBuffer = commandList.commandBuffer_,
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
    auto result = device_.resetFences(1, &fence);
    TOY_ASSERT(result == vk::Result::eSuccess);
    result = queue.submit2(1, &submitInfo, fence);
    TOY_ASSERT(result == vk::Result::eSuccess);
    result = device_.waitIdle();
    TOY_ASSERT(result == vk::Result::eSuccess);
}

Handle<Pipeline> VulkanRenderInterface::createPipelineInternal(
	const GraphicsPipelineDescriptor& descriptor, const std::vector<SetBindGroupMapping>& bindGroups, const std::vector<PushConstant>& pushConstants)
{

    const auto colorRenderTargets = static_cast<u32>(descriptor.renderTargetDescriptor.colorRenderTargets.size());
    auto colorRenderTargetFormats = std::vector<vk::Format>{};
    colorRenderTargetFormats.resize(colorRenderTargets);
    for (auto i = u32{}; i < colorRenderTargets; i++)
    {
        colorRenderTargetFormats[i] = mapColorFormat(descriptor.renderTargetDescriptor.colorRenderTargets[i].format);
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
        setLayouts[i] = bindGroupLayoutCache_[bindGroups[i].bindGroupLayout.index].layout;
    }

    const auto pushConstantsCount = static_cast<u32>(pushConstants.size());
    auto pushConstantRanges = std::vector<vk::PushConstantRange>{};
    pushConstantRanges.resize(pushConstantsCount);
    for (auto i = u32{}; i < pushConstantsCount; i++)
    {
        pushConstantRanges[i].offset = 0;
        pushConstantRanges[i].size = pushConstants[i].size;
        pushConstantRanges[i].stageFlags = vk::ShaderStageFlagBits::eAll;
    }

    const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo
    {
        .setLayoutCount = setCount,
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = pushConstantsCount,
        .pPushConstantRanges = pushConstantRanges.data()
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
    
    

    auto cullMode = vk::CullModeFlags{};
    switch (descriptor.state.faceCulling)
    {
    case FaceCull::front:
        cullMode = vk::CullModeFlagBits::eFront;
        break;
    case FaceCull::back:
        cullMode = vk::CullModeFlagBits::eBack;
        break;
    case FaceCull::none:
        cullMode = vk::CullModeFlagBits::eNone;
    }

    const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo
    {
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = cullMode,
        .frontFace = vk::FrontFace::eClockwise,
        .lineWidth = 1.0f
    };
    auto multisampleState = vk::PipelineMultisampleStateCreateInfo
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1
    };

    //TODO: there could be more then one color attachments
    auto colorAttachments = std::array
    {
        vk::PipelineColorBlendAttachmentState
        {
            .blendEnable = vk::Bool32{ false },
            .colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
        }
    };

    if (descriptor.state.blending == Blending::alphaBlend)
    {
        colorAttachments[0].blendEnable = vk::Bool32{ true };
        colorAttachments[0].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorAttachments[0].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorAttachments[0].colorBlendOp = vk::BlendOp::eAdd;
        colorAttachments[0].srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorAttachments[0].dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorAttachments[0].alphaBlendOp = vk::BlendOp::eAdd;
    }

    auto colorBlendState = vk::PipelineColorBlendStateCreateInfo
    {
        .attachmentCount = static_cast<uint32_t>(colorAttachments.size()),
        .pAttachments = colorAttachments.data(),
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

    auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo{};
    if(descriptor.state.depthTestEnabled)
    {
        depthStencilState.depthTestEnable = vk::Bool32{ descriptor.state.depthTestEnabled };
        depthStencilState.depthWriteEnable = vk::Bool32{ descriptor.state.depthTestEnabled };
        depthStencilState.depthCompareOp = vk::CompareOp::eLess;

        depthStencilState.depthBoundsTestEnable = vk::Bool32{ false };
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;
        depthStencilState.stencilTestEnable = vk::Bool32{ false };
    }

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
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayoutResult.value,
        },
        vk::PipelineRenderingCreateInfo
        {
            .viewMask = 0,
            .colorAttachmentCount = colorRenderTargets,
            .pColorAttachmentFormats = colorRenderTargetFormats.data(),
            .depthAttachmentFormat = hasDepthRenderTarget ? mapDepthFormat(descriptor.renderTargetDescriptor.depthRenderTarget.value().format) : vk::Format::eUndefined,
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
    
    {
        const auto& pools = descriptorPoolsPerFrame_[frame % maxDeferredFrames_];

        for(const auto& pool: pools)
        {
            device_.resetDescriptorPool(pool);
            //TODO: performance optimization: decide how many pools should be deleted for the next frame
        }
    }
}

Handle<Buffer> VulkanRenderInterface::createBufferInternal(
	const BufferDescriptor& descriptor, [[maybe_unused]] const DebugLabel label)
{
    
    const auto usage = vulkanMapBufferAccessUsageFlag(descriptor.accessUsage) | vk::BufferUsageFlagBits::eShaderDeviceAddress;

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

#ifdef TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
    setObjectName(device_, buffer, label.name);
    vmaSetAllocationName(allocator_, allocation, label.name); //TODO: not sure how it works!
#endif
    

    const auto handle = bufferStorage_.add(VulkanBuffer{ buffer, allocation }, descriptor);

	return handle;
}

auto VulkanRenderInterface::createVirtualTextureInternal(const VirtualTextureDescriptor& descriptor, const DebugLabel label) -> Handle<VirtualTexture>
{
    const auto imageType = vk::ImageType::e2D;
    const auto format = vk::Format::eB8G8R8A8Unorm;
    const auto extent = vk::Extent3D{ 8192, 8192, 1 };
    const auto mipLeveles = 13;
    const auto arrayLayers = 1;
    const auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    const auto tiling = vk::ImageTiling::eOptimal;
    const auto samples = vk::SampleCountFlagBits::e1;

    const auto queueFamalies = std::array{ VK_QUEUE_FAMILY_IGNORED };

    const auto createInfo = vk::ImageCreateInfo
    {
        .flags = vk::ImageCreateFlagBits::eSparseBinding | vk::ImageCreateFlagBits::eSparseResidency, //DODO: alos sparse aliasing?
        .imageType = imageType,
        .format = format,
        .extent = extent,
        .mipLevels = mipLeveles,
        .arrayLayers = arrayLayers,
        .samples = samples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive, //TODO: it could be wrong
        /*.queueFamilyIndexCount = static_cast<u32>(queueFamalies.size()),
        .pQueueFamilyIndices = queueFamalies.data(),*/
        .initialLayout = vk::ImageLayout::eUndefined
    };
    const auto result = device_.createImage(createInfo);

    const auto image = result.value;
    assert(image);
    const auto imageSparseMemoryRequirementsInfo = vk::ImageSparseMemoryRequirementsInfo2
    {
        .image = image
    };


    const auto sparseRequirements = device_.getImageSparseMemoryRequirements2(imageSparseMemoryRequirementsInfo);

    const auto formatInfo = vk::PhysicalDeviceSparseImageFormatInfo2
    {
        .format = format,
        .type = imageType,
        .samples = samples,
        .usage = usage,
        .tiling = tiling
    };

    const auto sparseFormatProperties = adapter_.getSparseImageFormatProperties2(formatInfo);

    const auto imageMemoryRequirementsInfo = vk::ImageMemoryRequirementsInfo2
    {
        .image = image
    };
    const auto imageMamoryRequirement = device_.getImageMemoryRequirements2(imageMemoryRequirementsInfo).memoryRequirements;

    const auto pageSize = imageMamoryRequirement.alignment;
    const auto pageCount = std::ceil<u32>((u32)(imageMamoryRequirement.size/(float)pageSize));

    auto pageMemoryRequirements = imageMamoryRequirement;
    pageMemoryRequirements.size = pageSize;
    

    const auto allocationCreateInfo = VmaAllocationCreateInfo
    {
        .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };

    auto allocations = std::vector<VmaAllocation>{};
    allocations.resize(pageCount);
    
    auto allocationInfos = std::vector<VmaAllocationInfo>{};
    allocationInfos.resize(pageCount);


    getMemoryRequirenments();
    const  auto allocationResult = vmaAllocateMemoryPages(allocator_, (VkMemoryRequirements*) & pageMemoryRequirements, &allocationCreateInfo, pageCount, allocations.data(), allocationInfos.data());

    return Handle<VirtualTexture>();
}

auto VulkanRenderInterface::createSamplerInternal(const SamplerDescriptor& descriptor, [[maybe_unused]] const DebugLabel label) -> Handle<Sampler>
{
  /*  VkSamplerCreateFlags    flags;
    VkFilter                magFilter;
    VkFilter                minFilter;
    VkSamplerMipmapMode     mipmapMode;
    VkSamplerAddressMode    addressModeU;
    VkSamplerAddressMode    addressModeV;
    VkSamplerAddressMode    addressModeW;
    float                   mipLodBias;
    VkBool32                anisotropyEnable;
    float                   maxAnisotropy;
    VkBool32                compareEnable;
    VkCompareOp             compareOp;
    float                   minLod;
    float                   maxLod;
    VkBorderColor           borderColor;
    VkBool32                unnormalizedCoordinates;*/

    const auto samplerCreateInfo = vk::SamplerCreateInfo
    {
        .magFilter = mapFilter(descriptor.magFilter),
        .minFilter = mapFilter(descriptor.minFilter),
        .mipmapMode = mapMipFilter(descriptor.mipFilter),
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .maxAnisotropy = 1.0f,
        .minLod = -1000,
        .maxLod = 1000
    };

    const auto result = device_.createSampler(samplerCreateInfo);
    TOY_ASSERT(result.result == vk::Result::eSuccess);

    const auto handle = samplerStorage_.add(VulkanSampler{ result.value }, descriptor);

    return handle;
}

auto VulkanRenderInterface::getMemoryRequirenments() -> void
{
    auto pageSizeX = u32{ 128 };
    auto pageSizeY = u32{ 128 };
    auto mipLevels = u32{ 9 };

    const auto samples = 8;

    auto reqs = std::vector<vk::MemoryRequirements2>{};
    reqs.resize(samples);

    for (auto i = 1; i <= samples; i++)
    {
        pageSizeX *= 1 << i;
        pageSizeX *= 1 << i;

        const auto testImageCreateInfo = vk::ImageCreateInfo
        {
            .flags = vk::ImageCreateFlagBits::eSparseBinding | vk::ImageCreateFlagBits::eSparseResidency,
            .imageType = vk::ImageType::e3D,
            .format = vk::Format::eBc7UnormBlock,
            .extent = vk::Extent3D{ pageSizeX, pageSizeY, 1 },
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eSampled
        };

        const auto imageMemoryRequirementsInfo = vk::DeviceImageMemoryRequirements
        {
            .pCreateInfo = &testImageCreateInfo,
        };
        const auto req = device_.getImageMemoryRequirements(imageMemoryRequirementsInfo);

        reqs[i - u64{ 1 }] = req;

        mipLevels++;
    }
    allocatePageMemoryInternal();
    
}

auto VulkanRenderInterface::allocatePageMemoryInternal() -> void
{
    auto pageMemoryReqrement = vk::MemoryRequirements
    {
        .size = 65536,
        .alignment = 65536,
        .memoryTypeBits = 0 //<- this value should be fetched correctly via one of vmaFind...MemoryTypeIndex() function, in this case it should target device local memory
    };

    

    const auto pageSize = pageMemoryReqrement.alignment;
    const auto pageCount = u32{ 2 };

    const auto minPagesCount = u32{ 4 };

    auto pool = VmaPool{};

    const auto memoryTypeIndex = pageMemoryReqrement.memoryTypeBits;

    const auto poolCreateInfo = VmaPoolCreateInfo
    {
        .memoryTypeIndex = u32{1} << memoryTypeIndex,
        .flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
        .blockSize = pageMemoryReqrement.alignment,
        .minBlockCount = minPagesCount,
        .minAllocationAlignment = pageMemoryReqrement.alignment
    };

    const auto result = vmaCreatePool(allocator_, &poolCreateInfo, &pool);

    TOY_ASSERT(vk::Result(result) == vk::Result::eSuccess);

    const auto allocationCreateInfo = VmaAllocationCreateInfo
    {
        .pool = pool
    };

    auto allocations = std::vector<VmaAllocation>{};
    allocations.resize(pageCount);

    auto allocationInfos = std::vector<VmaAllocationInfo>{};
    allocationInfos.resize(pageCount);

    const  auto allocationResult = vmaAllocateMemoryPages(allocator_, (VkMemoryRequirements*)&pageMemoryReqrement, &allocationCreateInfo, pageCount, allocations.data(), allocationInfos.data());


    for (const auto& allocationInfo : allocationInfos)
    {
        std::stringstream ss;
        ss << "Page | Pool";
        setObjectName(device_, (vk::DeviceMemory)allocationInfo.deviceMemory, ss.str().c_str());
    }

    auto statistics = VmaStatistics{};
    vmaGetPoolStatistics(allocator_, pool, &statistics);

    //TODO: it can be a good candidate for CVAR to call it from a debug console
    auto poolDetailedStatistics = VmaDetailedStatistics{};
    vmaCalculatePoolStatistics(allocator_, pool, &poolDetailedStatistics);

    auto totalStatistics = VmaTotalStatistics{};

    vmaCalculateStatistics(allocator_, &totalStatistics);
}

void VulkanRenderInterface::updateBindGroupInternal(
	const Handle<BindGroup>& bindGroup,
	const std::initializer_list<BindingDataMapping>& mappings)
{
    auto vulkanBindGroup = VulkanBindGroup{};
    //TODO: do something clever
    if(persistentBindGroupStorage_.contains(bindGroup))
    {
        vulkanBindGroup = persistentBindGroupStorage_.get(bindGroup);
    }
    else
    {
        vulkanBindGroup = bindGroupStorage_.get(bindGroup);
    }

    const auto mappingsVector = std::vector<BindingDataMapping>{ mappings };//TODO: smallvector


    auto descriptorWrites = std::vector<vk::WriteDescriptorSet>{}; //TODO: smallvector
    descriptorWrites.reserve(mappings.size());

    auto descriptorInfos = std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>>{};//TODO: smallvector
    descriptorInfos.resize(mappingsVector.size());

    for(auto i = u32{}; i < mappingsVector.size(); i++)
    {
        const auto& binding = mappingsVector[i];

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
        /*if(std::holds_alternative<CBV>(binding.view))
        {
            TOY_ASSERT(std::holds_alternative<CBV>(binding.view));
            
        }
        if (std::holds_alternative<UAV>(binding.view))
        {
            TOY_ASSERT(std::holds_alternative<UAV>(binding.view));
            
        }*/
    }

    for (auto i = u32{}; i < mappingsVector.size(); i++)
	{
        const auto& binding = mappingsVector[i];
        //TODO:: TEMP
        if(std::holds_alternative<CBV>(binding.view))
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
	const std::vector<SetBindGroupMapping>& bindGroups,
    const std::vector<PushConstant>& pushConstants)
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
        setLayouts[i] = bindGroupLayoutCache_[bindGroups[i].bindGroupLayout.index].layout;
    }

    const auto pushConstantsCount = static_cast<u32>(pushConstants.size());
    auto pushConstantRanges = std::vector<vk::PushConstantRange>{};
    pushConstantRanges.resize(pushConstantsCount);
    for (auto i = u32{}; i < pushConstantsCount; i++)
    {
        pushConstantRanges[i].offset = 0;
        pushConstantRanges[i].size = pushConstants[i].size;
        pushConstantRanges[i].stageFlags = vk::ShaderStageFlagBits::eCompute;
    }

    const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo
    {
        .setLayoutCount = setCount,
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = pushConstantsCount,
        .pPushConstantRanges = pushConstantRanges.data()
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

Handle<Image> VulkanRenderInterface::createImageInternal(
	const ImageDescriptor& descriptor)
{
    const auto usage = vulkanMapImageAccessUsageFlag(descriptor.accessUsage);

    auto queues = std::vector<u32>{};
    queues.reserve(3);

    if (descriptor.queuesSharing.containBit(QueuesSharing::graphics))
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

    auto imageType = vk::ImageType::e1D;
    if(descriptor.extent.height > 1)
    {
        imageType = vk::ImageType::e2D;
    }
    if (descriptor.extent.depth > 1)
    {
        imageType = vk::ImageType::e3D;
    }
   

    auto imageCreateInfo = vk::ImageCreateInfo
    {
        .imageType = imageType,
        .format = mapFormat(descriptor.format),
        .extent = mapExtent(descriptor.extent),
        .mipLevels = descriptor.mips,
        .arrayLayers = descriptor.layers,
        .samples = vk::SampleCountFlagBits::e1,//msaa
        .tiling = vk::ImageTiling::eOptimal,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    if (queues.size() > 1)
    {
        imageCreateInfo.sharingMode = vk::SharingMode::eConcurrent;
        imageCreateInfo.queueFamilyIndexCount = static_cast<u32>(queues.size());
        imageCreateInfo.pQueueFamilyIndices = queues.data();
    }

    //TODO: consider to use allocated pools
    const auto allocationCreateInfo = VmaAllocationCreateInfo
    {
        .usage = descriptor.memoryUsage == MemoryUsage::gpuOnly ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE : descriptor.memoryUsage == MemoryUsage::cpuOnly ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO
    };

    auto image = vk::Image{};
    auto allocation = VmaAllocation{};

    const auto result = static_cast<vk::Result>(vmaCreateImage(allocator_,
        reinterpret_cast<VkImageCreateInfo*>(&
            imageCreateInfo),
        &allocationCreateInfo,
        reinterpret_cast<VkImage*>(&image),
        &allocation,
        nullptr));

    TOY_ASSERT(result == vk::Result::eSuccess);

    const auto handle = imageStorage_.add(VulkanImage{ image, allocation }, descriptor);

    return handle;
}

Handle<ImageView> VulkanRenderInterface::createImageViewInternal(
	const ImageViewDescriptor& descriptor)
{
    //TODO: Mip levels????
    const auto imageViewCreateInfo = vk::ImageViewCreateInfo
    {
        .image = imageStorage_.get(descriptor.image).image,
        .viewType = mapViewType(descriptor.type),
        .format = mapFormat(descriptor.format),
        .subresourceRange = vk::ImageSubresourceRange
        {
            mapViewAspect(descriptor.aspect), 0, 1, 0, 1
        }
    };
    const auto result = device_.createImageView(imageViewCreateInfo);

    TOY_ASSERT(result.result == vk::Result::eSuccess);

    return imageViewStorage_.add(VulkanImageView{ result.value }, descriptor);
}



PerThreadCommandPoolData VulkanRenderInterface::initializePerRenderThreadData()
{
    auto graphicsPerFrame = std::vector<PerFrameCommandPoolData>(maxDeferredFrames_);
    auto asyncComputePerFrame = std::vector <PerFrameCommandPoolData>(maxDeferredFrames_);
    auto transferPerFrame = std::vector <PerFrameCommandPoolData>(maxDeferredFrames_);

    const auto graphicsIndex = queues_[QueueType::graphics].familyIndex;
    const auto asyncComputeIndex = queues_[QueueType::asyncCompute].familyIndex;
    const auto transferIndex = queues_[QueueType::transfer].familyIndex;

    constexpr auto graphicsCommandListPerFrame = u32{ 100 };//TODO:
    constexpr auto asyncComputeCommandListPerFrame = u32{ 10 };
    constexpr auto transferCommandListPerFrame = u32{ 10 };

    for (u32 i{}; i < maxDeferredFrames_; i++)
    {
        if constexpr (graphicsCommandListPerFrame != 0)
        {
            const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
            {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = graphicsIndex
            };

            {
                const auto result = device_.createCommandPool(commandPoolCreateInfo);
                TOY_ASSERT(result.result == vk::Result::eSuccess);
                graphicsPerFrame[i].commandPool = result.value;
            }
            const auto allocateInfo = vk::CommandBufferAllocateInfo
            {
                .commandPool = graphicsPerFrame[i].commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = graphicsCommandListPerFrame
            };
            {
                const auto result = device_.allocateCommandBuffers(allocateInfo);
                TOY_ASSERT(result.result == vk::Result::eSuccess);

                for(const auto& buffer : result.value)
                {
                    graphicsPerFrame[i].commandBuffers.push_back(buffer);
                }
                

                
            }
        }
        if constexpr (asyncComputeCommandListPerFrame != 0)
        {
            const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
            {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = asyncComputeIndex
            };

            asyncComputePerFrame[i].commandPool = device_.createCommandPool(commandPoolCreateInfo).value;

            const auto allocateInfo = vk::CommandBufferAllocateInfo
            {
                .commandPool = asyncComputePerFrame[i].commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = asyncComputeCommandListPerFrame
            };
            asyncComputePerFrame[i].commandBuffers = device_.allocateCommandBuffers(allocateInfo).value;
        }
        if constexpr (transferCommandListPerFrame != 0)
        {
            const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo
            {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = transferIndex
            };

            transferPerFrame[i].commandPool = device_.createCommandPool(commandPoolCreateInfo).value;

            const auto allocateInfo = vk::CommandBufferAllocateInfo
            {
                .commandPool = transferPerFrame[i].commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = transferCommandListPerFrame
            };
            transferPerFrame[i].commandBuffers = device_.allocateCommandBuffers(allocateInfo).value;
        }
    }


    PerThreadCommandPoolData perThreadData
    {};
    if constexpr (graphicsCommandListPerFrame != 0)
    {
        perThreadData.perQueueType[QueueType::graphics] = graphicsPerFrame;
    }
    if constexpr (asyncComputeCommandListPerFrame != 0)
    {
        perThreadData.perQueueType[QueueType::asyncCompute] = asyncComputePerFrame;
    }
    if constexpr (transferCommandListPerFrame != 0)
    {
        perThreadData.perQueueType[QueueType::transfer] = transferPerFrame;
    }

    return perThreadData;
}


void VulkanRenderInterface::beginDebugLabelInternal(const QueueType queueType, const DebugLabel& label)
{
    const auto debugLabel = vk::DebugUtilsLabelEXT
    {
    	.pLabelName = label.name,
        .color = std::array{ label.color.r(), label.color.g(), label.color.b(), 1.0f}
    };
    queues_[queueType].queue.beginDebugUtilsLabelEXT(debugLabel);
}

void VulkanRenderInterface::endDebugLabelInternal(const QueueType queueType)
{
    queues_[queueType].queue.endDebugUtilsLabelEXT();
}