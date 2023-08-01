set(MODULE_NAME Editor)


set(MODULE_SOURCES 
	Source/EditorApplication.cpp
	Source/MaterialEditor.cpp
	Source/MaterialEditorResolver.cpp
	Source/MaterialEditorGlslResolver.cpp
	Source/MaterialEditorNode.cpp
	Source/MaterialEditorScalarNode.cpp
	Source/MaterialEditorColorNode.cpp
	Source/MaterialEditorArithmeticNode.cpp
	Source/MaterialModel.cpp
	 Source/AssetDatabase.cpp
	 Source/AssetBrowser.cpp
	 Source/AssetDropWidget.cpp
)

set(MODULE_HEADERS
	Source/EditorApplication.h
	Source/MaterialEditor.h
	Source/Editor.h
	Source/MaterialEditorResolver.h
	Source/MaterialEditorGlslResolver.h
	Source/MaterialEditorNode.h
	Source/MaterialEditorScalarNode.h
	Source/MaterialEditorColorNode.h
	Source/MaterialEditorArithmeticNode.h
	Source/Undo.h
	Source/MaterialNodeRegistry.h
	Source/MaterialModel.h
	Source/AssetDatabase.h
	Source/AssetBrowser.h
	Source/AssetDropWidget.h
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
		SDLWindow
		RenderInterface
		GLSLRuntimeCompiler
		GraphicsDebugger
		MPMCQueue::MPMCQueue
		Freetype::Freetype
		imgui::imgui
		imgui-node-editor-static
		Runtime
		nlohmann_json::nlohmann_json
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	get_property(RESOURCES GLOBAL "${RESOURCE}" PROPERTY TOY_RESOURCE_DIR)
	if(EXISTS ${RESOURCES})
	set_property(GLOBAL PROPERTY TOY_RESOURCE_DIR ${RESOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	else()
	set_property(GLOBAL PROPERTY TOY_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
	endif()
endif()

if(BUILD_TOY_ENGINE_MODULE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CMakeLists.txt")
    message(STATUS "Build tests for module ${MODULE_NAME}")
    add_subdirectory(Tests)   
endif()