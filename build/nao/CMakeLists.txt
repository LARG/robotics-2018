cmake_minimum_required(VERSION 2.8)

project(naointerface)

include(../common.cmake)

file(GLOB_RECURSE SRCS 
  ${INTERFACE_DIR}/nao/*.cpp
  ${INTERFACE_DIR}/nao/*.h
)


qi_create_lib(naointerface SHARED ${SRCS})
TARGET_LINK_LIBRARIES(naointerface ${LIBCORE} ${LIBYUVIEW})
qi_use_lib(naointerface ALCOMMON ALMEMORYFASTACCESS ALAUDIO ALPROXIES opencv2_core opencv2_highgui)
qi_stage_lib(naointerface)

