include(GoogleTest)
add_executable(${MODULE_NAME}_tests
    ShaderCompilationTest.cpp
)
gtest_add_tests(TARGET ${MODULE_NAME}_tests)
               
target_include_directories(${MODULE_NAME}_tests
    PUBLIC
        "../"
)

target_link_libraries(${MODULE_NAME}_tests
    PRIVATE 
        GTest::gmock_main
        GTest::gtest
        GTest::gtest_main
        Core
        GLSLRuntimeCompiler
)

set_target_properties(${MODULE_NAME}_tests PROPERTIES FOLDER Tests)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CopyResources.cmake" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
    add_custom_target(${MODULE_NAME}_tests_copy_resource_files
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/CopyResources.cmake
        COMMENT "Copy resources for ${MODULE_NAME}_tests from ${CMAKE_CURRENT_SOURCE_DIR} to ${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_dependencies(${MODULE_NAME}_tests ${MODULE_NAME}_tests_copy_resource_files)
    set_target_properties(${MODULE_NAME}_tests_copy_resource_files PROPERTIES FOLDER Misc)
endif()

set_property(TARGET ${MODULE_NAME}_tests PROPERTY CXX_STANDARD 20)