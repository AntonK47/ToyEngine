set(APPLICATION_NAME MeshBuilder)
set(MODULE_SOURCES 
	main.cpp
	Mesh.h
	MeshBuilder.h
	MeshletBuilder.h
	MeshletBuilder.cpp
)

set(RESOURCE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")

add_executable(${APPLICATION_NAME}
		${MODULE_SOURCES}
)

find_package(meshoptimizer CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(${APPLICATION_NAME}
	PRIVATE
		Engine
		meshoptimizer::meshoptimizer
		assimp::assimp
		Boost::program_options
)