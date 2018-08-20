cmake_minimum_required(VERSION 2.8)

project(memory_test)

include(../common.cmake)

set(SRCS
  ${INTERFACE_DIR}/memory_test/main.cpp
  ${SRC_DIR}/memory/MemoryBlock.cpp
  ${SRC_DIR}/memory/SharedMemory.cpp
  ${SRC_DIR}/memory/StreamBuffer.cpp
)

qi_create_bin(memory_test ${SRCS})
target_link_libraries(memory_test pthread dl rt ${LINK_LIBS})
