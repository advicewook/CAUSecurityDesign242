# FindMySQLConnectorCpp.cmake

find_path(MySQLConnectorCpp_INCLUDE_DIR
  NAMES mysql_connection.h
  PATHS "C:/Program Files/MySQL/mysql-connector-c++-9.1.0-winx64/include"
)

find_library(MySQLConnectorCpp_LIBRARY
  NAMES mysqlcppconn8
  PATHS "C:/Program Files/MySQL/mysql-connector-c++-9.1.0-winx64/lib64"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQLConnectorCpp DEFAULT_MSG MySQLConnectorCpp_INCLUDE_DIR MySQLConnectorCpp_LIBRARY)

if (MySQLConnectorCpp_FOUND)
  set(MySQLConnectorCpp_INCLUDE_DIRS ${MySQLConnectorCpp_INCLUDE_DIR})
  set(MySQLConnectorCpp_LIBRARIES ${MySQLConnectorCpp_LIBRARY})
endif()
