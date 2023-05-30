set(APPLICATION_NAME SampleApplication)
set(MODULE_SOURCES 
	main.cpp
)

set(${APPLICATION_NAME}_RESOURCE_DIRECTORY_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Resources")

add_executable(${APPLICATION_NAME}
		${MODULE_SOURCES}
)

target_link_libraries(${APPLICATION_NAME}
	PRIVATE
		Runtime
)

set_property(TARGET ${APPLICATION_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)


set_property(TARGET ${APPLICATION_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

get_property(RESOURCES GLOBAL "${RESOURCE}" PROPERTY TOY_RESOURCE_DIR)

set(commands)

foreach(RESOURCE_DIR ${RESOURCES})
  list(APPEND commands
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
                ${RESOURCE_DIR}
                ${SampleApplication_RESOURCE_DIRECTORY_DESTINATION})
endforeach()

add_custom_target(${APPLICATION_NAME}_copy_resource_files ALL COMMAND
${commands}
)

add_dependencies(${APPLICATION_NAME} ${APPLICATION_NAME}_copy_resource_files)
   set_target_properties(${APPLICATION_NAME}_copy_resource_files PROPERTIES FOLDER Misc)