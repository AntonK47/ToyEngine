set(APPLICATION_NAME SampleApplication)
set(MODULE_SOURCES 
	main.cpp
)

add_executable(${APPLICATION_NAME}
		${MODULE_SOURCES}
)


target_link_libraries(${APPLICATION_NAME}
	PRIVATE
		Engine
)