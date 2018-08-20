cmake_minimum_required(VERSION 2.8)

project(siminterface)

include(../common.cmake)

file(GLOB_RECURSE SRCS 
  ${INTERFACE_DIR}/sim_spark/*.cpp
  ${INTERFACE_DIR}/sim_spark/*.c
  ${INTERFACE_DIR}/sim_spark/*.h
)

INCLUDE_DIRECTORIES(/usr/local/include/simspark /usr/include/simspark)

link_directories(/usr/lib/simspark /usr/lib/simspark/rcssnet /usr/local/lib/simspark /usr/local/lib/simspark/rcssnet /usr/lib32/simspark /usr/lib/i386-linux-gnu/)  # For some reason this has to be before the add_execuatble

qi_create_bin(siminterface ${SRCS})

qi_use_lib(siminterface core) # ALCOMMON ALMEMORYFASTACCESS)
target_link_libraries(siminterface boost_thread-mt rcssnet3D dl rt)
#target_link_libraries(naointerface ${LINK_LIBS})
