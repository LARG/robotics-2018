cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR} ${RSWALK2014_DIR}) # ${CMAKE_CURRENT_SOURCE_DIR}) - Commented this out because redundant - Josiah

add_definitions(-DLINUX -DRELEASE -DTARGET_ROBOT)


#if(CMAKE_TOOLCHAIN_FILE)
#  include("${CMAKE_CURRENT_SOURCE_DIR}/libagent.cmake")
#endif(CMAKE_TOOLCHAIN_FILE)

include("${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/bootstrap-eigen.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/runswift.cmake")

#install configs
if( NOT OFFLINE )
set( RSWALK_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config )
install( DIRECTORY ${RSWALK_CONFIG_DIR} 
         DESTINATION ${CMAKE_INSTALL_PREFIX} )
endif()


#if(CMAKE_TOOLCHAIN_FILE)
#  include("${CMAKE_CURRENT_SOURCE_DIR}/robot.cmake")
#else(CMAKE_TOOLCHAIN_FILE)
  #simrobot has been removed
#endif(CMAKE_TOOLCHAIN_FILE)

#include("${CMAKE_CURRENT_SOURCE_DIR}/testrunswift.cmake")
#include("${CMAKE_CURRENT_SOURCE_DIR}/benchrunswift.cmake")
#cotire(agent robot-static runswift soccer-static soccer)
