#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Logger::logger_cpp" for configuration ""
set_property(TARGET Logger::logger_cpp APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Logger::logger_cpp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/liblogger_cpp.a"
  )

list(APPEND _cmake_import_check_targets Logger::logger_cpp )
list(APPEND _cmake_import_check_files_for_Logger::logger_cpp "${_IMPORT_PREFIX}/lib64/liblogger_cpp.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
