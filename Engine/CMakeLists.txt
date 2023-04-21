set(TOY_ENGINE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_UNITY_BUILD ON)
foreach(MODULE 
	Core 
	Engine
	Window
	SDLWindow
	Win32Window
	RenderInterface 
	GLSLRuntimeCompiler
	GraphicsDebugger
)
	
	message(STATUS "Prepare module ${MODULE}")
	add_subdirectory(${MODULE} EXCLUDE_FROM_ALL)
	
	#DOTO: if sanitizer enable
	#target_compile_options(${MODULE} PRIVATE /fsanitize=address)
	#target_link_options(${MODULE} PRIVATE /fsanitize=address)
	#target_compile_definitions(${MODULE} PRIVATE _DISABLE_VECTOR_ANNOTATION)
endforeach()
