set(MODULE_NAME RenderInterface)
set(MODULE_SOURCES 
	Source/RenderInterface.cpp
)
	"Source/CommandListValidator.h"
	"Source/CommandListValidator.cpp")
set(MODULE_HEADERS 
	Source/RenderInterface.h
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
		Window
)