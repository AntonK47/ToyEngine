set(MODULE_NAME GraphicsDebuger)
set(MODULE_SOURCES 
	Source/RenderDocCapture.cpp
)
set(MODULE_HEADERS 
	Source/RenderDocCapture.h
	Source/renderdoc_app.h
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


find_package(glm CONFIG REQUIRED)
target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
		VulkanRenderInterface
		
)
#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
#	set_property(GLOBAL PROPERTY TOY_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
#endif()
