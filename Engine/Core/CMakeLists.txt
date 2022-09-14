set(MODULE_NAME Core)



add_library(${MODULE_NAME}
	INTERFACE
		
)

target_include_directories(${MODULE_NAME} 
	INTERFACE
		Source/ 
)