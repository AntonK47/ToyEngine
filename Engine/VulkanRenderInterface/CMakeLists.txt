set(MODULE_NAME VulkanRenderInterface)
set(MODULE_HEADERS 
	Source/VulkanCommandList.h
	Source/VulkanRenderInterface.h
	"Source/Vulkan.h"
	Source/Structs.h
	Source/UploadBufferRing.h
 "Source/VulkanBindGroupAllocator.h" "Source/VulkanMappings.h" "Source/FrameAllocator.h")

set(MODULE_SOURCES
	Source/VulkanRenderInterface.cpp
	Source/Vulkan.cpp
	Source/UploadBufferRing.cpp
 "Source/VulkanCommandList.cpp" 
 "Source/VulkanBindGroupAllocator.cpp" "Source/VulkanMappings.h" "Source/VulkanMappings.cpp")

add_library(${MODULE_NAME} 
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
)
#find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/
	PRIVATE
#		${VULKAN_HPP_INCLUDE_DIRS}
)

find_package(Vulkan REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

target_link_libraries(${MODULE_NAME}
	PUBLIC
		Core
		Window
		RenderInterface
	#PRIVATE
		unofficial::vulkan-memory-allocator::vulkan-memory-allocator
		
	
		Vulkan::Vulkan
		#Vulkan::Headers
)

