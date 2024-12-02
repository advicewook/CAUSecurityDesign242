# FindMySQL.cmake
find_path(MySQL_INCLUDE_DIR
  NAMES mysql.h
  PATHS ${MySQL_DIR}/include
)

find_library(MySQL_LIBRARY
  NAMES mysqlclient
  PATHS ${MySQL_DIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQL DEFAULT_MSG MySQL_INCLUDE_DIR MySQL_LIBRARY)

if (MySQL_FOUND)
  set(MySQL_INCLUDE_DIRS ${MySQL_INCLUDE_DIR})
  set(MySQL_LIBRARIES ${MySQL_LIBRARY})
endif()