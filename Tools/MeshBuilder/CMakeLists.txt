set(APPLICATION_NAME MeshBuilder)
set(MODULE_SOURCES 
	main.cpp
	Mesh.h
	MeshBuilder.h
	MeshletBuilder.h
	MeshletBuilder.cpp
	MeshBuilder.cpp)

set(RESOURCE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")

add_library(${APPLICATION_NAME} 
	STATIC
		${MODULE_SOURCES}
)

target_include_directories(${APPLICATION_NAME} 
	PUBLIC
		/
)

find_package(meshoptimizer CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

target_link_libraries(${APPLICATION_NAME}
	PRIVATE
		Core
		Engine
		meshoptimizer::meshoptimizer
		assimp::assimp
)

set_property(TARGET ${APPLICATION_NAME} PROPERTY CXX_STANDARD 20)