#refer to https://github.com/jochenklar/daiquiri/blob/3273126c065356ec5e39ff858b9fb1df7b7da5a3/library/paqu/mysql_node_jobMon/CMakeLists.txt

cmake_minimum_required (VERSION 2.6)
project (binlog_dump_list)

OPTION(WITH_DEBUG "Use dbug/safemutex" OFF)
OPTION(WITH_PLUGIN_DEBUG "Print debug info of plugin" OFF)
OPTION(FOR_OFFICIAL_MYSQL "Use options of official Mysql, instead of detecting MYSQL_PATH" OFF)

#SET PATH HERE
set(MYSQL_PATH "/opt/mysql")
set(MYSQL_SOURCES_PATH "/vagrant/mysql-5.5.35")

IF(MYSQL_PATH)
    set(MYSQL_CONFIG "${MYSQL_PATH}/bin/mysql_config")
ELSE()
    find_file(MYSQL_CONFIG mysql_config)
ENDIF()

execute_process(COMMAND ${MYSQL_CONFIG} --include OUTPUT_VARIABLE MYSQL_INCLUDE_DIR)
execute_process(COMMAND ${MYSQL_CONFIG} --plugindir OUTPUT_VARIABLE MYSQL_PLUGIN_DIR)

IF(FOR_OFFICIAL_MYSQL)
	SET(MYSQL_LIBRARIES "-lpthread -lm -lrt -ldl")
	SET(MYSQL_CFLAGS "-g -pipe -m64  -fPIC -g -static-libgcc -fno-omit-frame-pointer -fno-strict-aliasing -DMY_PTHREAD_FASTMUTEX=1")
ELSE()
	execute_process(COMMAND ${MYSQL_CONFIG} --libs OUTPUT_VARIABLE MYSQL_LIBRARIES)
	execute_process(COMMAND ${MYSQL_CONFIG} --clags OUTPUT_VARIABLE MYSQL_CFLAGS)
ENDIF()

STRING(REGEX REPLACE "-lmysqlclient" "" MYSQL_LIBRARIES ${MYSQL_LIBRARIES})
STRING(REGEX REPLACE "\n" "" MYSQL_INCLUDE_DIR ${MYSQL_INCLUDE_DIR})
STRING(REGEX REPLACE "\n" "" MYSQL_LIBRARIES ${MYSQL_LIBRARIES})
STRING(REGEX REPLACE "\n" "" MYSQL_PLUGIN_DIR ${MYSQL_PLUGIN_DIR})

SET(BUILD_SHARED_LIBS ON) #-shared

IF(WITH_DEBUG)
	SET(CMAKE_BUILD_TYPE Debug)
	add_definitions(-DDBUG_ON)
	add_definitions(-DSAFE_MUTEX)
ELSE()
	add_definitions(-DDBUG_OFF)
ENDIF()

IF(WITH_PLUGIN_DEBUG)
	add_definitions(-DDEBUG_PLUGIN)
ENDIF()

add_definitions(-DMYSQL_DYNAMIC_PLUGIN)
add_definitions(-DMYSQL_SERVER)
add_definitions(-DDISABLE_DTRACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_INCLUDE_DIR} -fPIC -fno-exceptions -fno-rtti ${MYSQL_CFLAGS}")

include_directories ("${PROJECT_SOURCE_DIR}" "${MYSQL_SOURCES_PATH}/include" "${MYSQL_SOURCES_PATH}/mysys" "${MYSQL_SOURCES_PATH}/regex" "${MYSQL_SOURCES_PATH}/sql" "${MYSQL_SOURCES_PATH}")


file(GLOB FILES_SRC "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.cc")

link_directories(${MYSQL_SOURCES_PATH}/libservices)
add_library (binlog_dump_list ${FILES_SRC})
TARGET_LINK_LIBRARIES(binlog_dump_list mysqlservices)

#get rid of the lib infront of the target file name
set_target_properties(binlog_dump_list PROPERTIES PREFIX "")

INSTALL(TARGETS binlog_dump_list DESTINATION "${MYSQL_PLUGIN_DIR}")