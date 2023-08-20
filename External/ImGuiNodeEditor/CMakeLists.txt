include(FetchContent)
#imgui-node-editor  
FetchContent_Declare(imgui-node-editor
    GIT_REPOSITORY https://github.com/AntonK47/imgui-node-editor.git
    #https://github.com/ocornut/imgui_node_editor.git
    GIT_TAG origin/develop#origin/fix-for-v189.4 #fix-for-v189.4
)

FetchContent_MakeAvailable(imgui-node-editor)
add_library(
    imgui-node-editor-static 
STATIC 
    ${imgui-node-editor_SOURCE_DIR}/crude_json.h
    ${imgui-node-editor_SOURCE_DIR}/crude_json.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_bezier_math.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_bezier_math.inl
    ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_extra_math.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_extra_math.inl
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_api.cpp
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_internal.h
    ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_internal.inl)
        
    target_link_libraries(
    imgui-node-editor-static
PRIVATE
    imgui::imgui)

target_include_directories(imgui-node-editor-static 
PUBLIC
    ${imgui-node-editor_SOURCE_DIR}
)

set_property(TARGET imgui-node-editor-static PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

install(TARGETS imgui-node-editor-static LIBRARY DESTINATION lib)


set_target_properties(imgui-node-editor-static PROPERTIES FOLDER Misc)