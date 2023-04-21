set(MODULE_NAME Win32Window)
set(MODULE_HEADERS 
	Source/Win32Window.h

)
set(MODULE_SOURCES
	Source/Win32Window.cpp
	Source/Win32EventPolling.cpp
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
	PUBLIC
		Window
)