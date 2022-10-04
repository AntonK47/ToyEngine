set(MODULE_NAME Engine)
set(MODULE_SOURCES 
	Source/Application.cpp
)
set(MODULE_HEADERS 
	Source/Application.h
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
		SDLWindow
		glm::glm
		VulkanRenderInterface
		GLSLRuntimeCompiler
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CopyResources.cmake" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
    add_custom_target(${MODULE_NAME}_copy_resource_files
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/CopyResources.cmake
        COMMENT "Copy resources for ${MODULE_NAME} from ${CMAKE_CURRENT_SOURCE_DIR} to ${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_dependencies(${MODULE_NAME} ${MODULE_NAME}_copy_resource_files)
    set_target_properties(${MODULE_NAME}_copy_resource_files PROPERTIES FOLDER Misc)
endif()