set(MODULE_NAME VulkanRenderInterface)
set(MODULE_HEADERS 
	Source/VulkanCommandList.h
	Source/VulkanRenderInterface.h
	Source/Vulkan.h
	Source/Structs.h
	Source/UploadBufferRing.h
	Source/VulkanBindGroupAllocator.h
	Source/VulkanMappings.h
	Source/FrameAllocator.h)

set(MODULE_SOURCES
	Source/VulkanRenderInterface.cpp
	Source/Vulkan.cpp
	Source/UploadBufferRing.cpp
	Source/VulkanCommandList.cpp
	Source/VulkanBindGroupAllocator.cpp
	Source/VulkanMappings.h
	Source/VulkanMappings.cpp)

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
	PUBLIC 
		unofficial::vulkan-memory-allocator::vulkan-memory-allocator
		Vulkan::Vulkan
		RenderInterface
)