set(MODULE_NAME Core)

set(MODULE_SOURCES 
	Source/Hash.cpp
 "Source/Logger.cpp" "Source/Scene.cpp")
set(MODULE_HEADERS 
	Source/CommonTypes.h
	Source/Hash.h
 "Source/Common.h" "Source/Logger.h" "Source/Scene.h")


add_library(${MODULE_NAME}
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
)

target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/ 
)

find_package(folly CONFIG REQUIRED)

target_link_libraries(${MODULE_NAME}
	PUBLIC
		cityhash
		g3log
		Folly::folly
)