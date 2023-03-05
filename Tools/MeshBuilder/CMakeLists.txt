set(APPLICATION_NAME MeshBuilder)
set(MODULE_SOURCES 
	main.cpp
	Mesh.h
	MeshBuilder.h
	MeshletBuilder.h
	MeshletBuilder.cpp
	MeshBuilder.cpp)

set(RESOURCE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")

find_package(meshoptimizer CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(${APPLICATION_NAME}
		${MODULE_SOURCES}
)

target_link_libraries(${APPLICATION_NAME}
	PRIVATE
		Core
		Engine
		meshoptimizer::meshoptimizer
		assimp::assimp
)

set_property(TARGET ${APPLICATION_NAME} PROPERTY CXX_STANDARD 20)