set(MODULE_NAME Core)

set(MODULE_SOURCES 
	Source/Hash.cpp
	Source/Logger.cpp
	Source/Scene.cpp)
set(MODULE_HEADERS 
	
	Source/Hash.h
	Source/Core.h
	Source/Logger.h
	Source/Scene.h 
	Source/DebugHelper.h)

find_path(ZPP_BITS_INCLUDE_DIRS "zpp_bits.h")

add_library(${MODULE_NAME}
	STATIC
		${MODULE_SOURCES}
		${MODULE_HEADERS}
)

target_include_directories(${MODULE_NAME} 
	PUBLIC
		Source/ 
	PRIVATE ${ZPP_BITS_INCLUDE_DIRS}
)


target_link_libraries(${MODULE_NAME}
	PRIVATE
		cityhash
		g3log
	PUBLIC
		glm::glm

)