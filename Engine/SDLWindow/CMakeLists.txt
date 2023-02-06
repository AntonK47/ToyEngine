set(MODULE_NAME SDLWindow)
set(MODULE_HEADERS 
	Source/SDLWindow.h

)
set(MODULE_SOURCES
	Source/SDLWindow.cpp
	Source/SDLEventPolling.cpp
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
		SDL2::SDL2
	PUBLIC
		Window
)