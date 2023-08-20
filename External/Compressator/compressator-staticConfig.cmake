add_library(compressonator-static STATIC IMPORTED)
set_target_properties(compressonator-static PROPERTIES
  IMPORTED_LOCATION_DEBUG "C:/Compressonator_4.3.206//lib/VS2019/x64/CMP_Framework_MTd.lib"
  IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
  IMPORTED_LOCATION "C:/Compressonator_4.3.206//lib/VS2019/x64/CMP_Framework_MT.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/Compressonator_4.3.206/include"
)