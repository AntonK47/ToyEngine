include(FetchContent)

FetchContent_Declare(IconFontCppHeaders
    GIT_REPOSITORY https://github.com/juliettef/IconFontCppHeaders
    GIT_TAG origin/main
)

FetchContent_MakeAvailable(IconFontCppHeaders)

add_library(iconsHeaders INTERFACE 
    ${iconfontcppheaders_SOURCE_DIR}/IconsFontAwesome6.h
    ${iconfontcppheaders_SOURCE_DIR}/IconsFontAwesome6Brands.h)
target_include_directories(iconsHeaders INTERFACE ${iconfontcppheaders_SOURCE_DIR})

#add_library(IconFontCppHeaders INTERFACE IMPORTED)
#set_property(TARGET IconFontCppHeaders PROPERTY
#  INTERFACE_INCLUDE_DIRECTORIES .)