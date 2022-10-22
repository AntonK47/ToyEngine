set(TOY_ENGINE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(MODULE 
	Core 
	Engine
	Window
	SDLWindow
	RenderInterface 
	VulkanRenderInterface
	GLSLRuntimeCompiler
	GraphicsDebugger
)
	message(STATUS "Prepare module ${MODULE}")
	add_subdirectory(${MODULE} EXCLUDE_FROM_ALL)
endforeach()
