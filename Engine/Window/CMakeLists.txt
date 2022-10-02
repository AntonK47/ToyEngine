set(MODULE_NAME Window)
set(MODULE_HEADERS 
	Source/Window.h
	Source/WindowIO.h

)
set(MODULE_SOURCES
	Source/IOStateReset.cpp 
	"Source/Window.cpp")

add_library(${MODULE_NAME} 
	STATIC
		${MODULE_HEADERS}
		${MODULE_SOURCES}
)

target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/
)

target_link_libraries(${MODULE_NAME}
	PRIVATE
		Core
)