set(MODULE_NAME RenderInterface)
set(MODULE_SOURCES 
	Source/CommandListValidator.cpp
	Source/CommandList.cpp
	Source/RenderInterface.cpp
	Source/RenderInterfaceValidator.cpp
	"Source/VulkanRHI/Vulkan.cpp"
	"Source/VulkanRHI/VulkanCommandList.cpp"
	"Source/VulkanRHI/VulkanMappings.cpp"
	"Source/VulkanRHI/VulkanRenderInterface.cpp"
)
set(MODULE_HEADERS 
	Source/CommandList.h
	Source/CommandListBase.h
	Source/CommandListValidator.h
	Source/Handle.h
	Source/QueueType.h
	Source/RenderInterface.h
	Source/RenderInterfaceBase.h
	Source/RenderInterfaceTypes.h
	Source/RenderInterfaceValidator.h
	Source/SubmitBatch.h
	Source/SubmitBatchBase.h
	Source/SubmitDependency.h
	Source/ValidationCommon.h 
	Source/VirtualTextureStreaming.h
	"Source/VulkanRHI/Vulkan.h"
	"Source/VulkanRHI/VulkanCommandList.h"
	"Source/VulkanRHI/VulkanMappings.h"
	"Source/VulkanRHI/VulkanRenderInterface.h"
	"Source/VulkanRHI/VulkanRenderInterfaceTypes.h"
	"Source/VulkanRHI/VulkanSubmitBatch.h"
)

add_library(${MODULE_NAME}
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
    )


target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/
)

target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
		Window
		glm::glm
		PUBLIC 
		unofficial::vulkan-memory-allocator::vulkan-memory-allocator
		Vulkan::Vulkan
)

#set_property(TARGET ${MODULE_NAME} PROPERTY CXX_STANDARD 23)