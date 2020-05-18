#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "rpclib::rpc" for configuration ""
set_property(TARGET rpclib::rpc APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(rpclib::rpc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/librpc.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS rpclib::rpc )
list(APPEND _IMPORT_CHECK_FILES_FOR_rpclib::rpc "${_IMPORT_PREFIX}/lib/librpc.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
