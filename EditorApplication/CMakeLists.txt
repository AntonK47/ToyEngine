set(APPLICATION_NAME ToyEngineEditor)
set(MODULE_SOURCES 
	main.cpp
)

set(RESOURCE_DIRECTORY_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Resources")

add_executable(${APPLICATION_NAME}
		${MODULE_SOURCES}
)

target_link_libraries(${APPLICATION_NAME}
	PRIVATE
		Editor
)

set_property(TARGET ${APPLICATION_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)


set_property(TARGET ${APPLICATION_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

get_property(RESOURCES GLOBAL "${RESOURCE}" PROPERTY TOY_RESOURCE_DIR)

foreach(RESOURCE_DIR ${RESOURCES})
    #message("Copy resources for ${MODULE_NAME} from ${RESOURCE_DIR} to ${RESOURCE_DIRECTORY_DESTINATION}")
	    add_custom_target(${APPLICATION_NAME}_copy_resource_files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                ${RESOURCE_DIR}
                ${RESOURCE_DIRECTORY_DESTINATION}
        COMMENT "Copy resources for ${APPLICATION_NAME} from ${RESOURCE_DIR} to ${RESOURCE_DIRECTORY_DESTINATION}"
    )

    add_dependencies(${APPLICATION_NAME} ${APPLICATION_NAME}_copy_resource_files)
    set_target_properties(${APPLICATION_NAME}_copy_resource_files PROPERTIES FOLDER Misc)
	
	#add_custom_command(
    #    TARGET ${APPLICATION_NAME} POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy_directory
    #            ${RESOURCE_DIR}
    #            ${RESOURCE_DIRECTORY_DESTINATION})
	#file(COPY "${RESOURCE_DIR}" DESTINATION ${RESOURCE_DIRECTORY_DESTINATION})
endforeach()