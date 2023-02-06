set(MODULE_NAME GraphicsDebugger)
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

target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
		Window
		VulkanRenderInterface
		
)