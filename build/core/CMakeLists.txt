CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(core)

INCLUDE($ENV{NAO_HOME}/build/common.cmake)
INCLUDE_DIRECTORIES(${BHWALK2013_DIR} ${BHWALK2011_DIR} ${RSWALK2014_DIR})
IF(${CAFFE_BUILD})
  INCLUDE_DIRECTORIES(${CAFFE_INCLUDE})
ENDIF(${CAFFE_BUILD})

SET(OPERATIONS_H ${NAO_HOME}/core/memory/MemoryBlockOperations.h)
SET(OPERATIONS_PY ${NAO_HOME}/build/core/generate_block_operations.py)
ADD_CUSTOM_COMMAND(
  OUTPUT ${OPERATIONS_H}
  COMMAND ${OPERATIONS_PY} ${OPERATIONS_H}
  DEPENDS ${OPERATIONS_PY}
)
ADD_CUSTOM_TARGET(operations DEPENDS ${OPERATIONS_H})

SET(IGNORE_REGEXS
  "core/vision/deprecated/*"
  "common/fields/*"
  "memory/schema/internal/*"
  "python/PythonInterface.*"
  ".*backup.*"
  ".*bak.*"
  "${BHWALK2011_DIR}/*"
  "${BHWALK2013_DIR}/*"
  "${RSWALK2014_DIR}/*"
)

# collect all the files into a temp var
FILE(GLOB_RECURSE TEMP_SRCS 
  ${SRC_DIR}/*.cpp
  ${SRC_DIR}/*.h
)

SET(SRCS ${SWIG_SRCS})

# remove files from the excluded dirs and fill out the SRCS var
FOREACH(item ${TEMP_SRCS})
  set(excluded False)
  FOREACH(ignoreRegex ${IGNORE_REGEXS})
    STRING(REGEX MATCH ${ignoreRegex} excludedTemp ${item})
    IF (excludedTemp)
      set(excluded True)
      #message("IGNORE_REGEX: " ${ignoreRegex} " ignoring " ${item})
    ENDIF(excludedTemp)
  ENDFOREACH(ignoreRegex ${IGNORE_REGEXS})
  IF(NOT excluded)
    list(APPEND SRCS ${item})
  ENDIF(NOT excluded)
ENDFOREACH(item ${TEMP_SRCS})

INCLUDE(${NAO_HOME}/build/schema/build.cmake)
SET(SRCS ${SRCS} ${GENERATED_SCHEMA_SOURCES})

IF(TOOL_BUILD)
  ADD_LIBRARY(core STATIC ${SRCS})
ELSE(TOOL_BUILD)
  qi_create_lib(core STATIC ${SRCS})
  qi_stage_lib(core)
ENDIF(TOOL_BUILD)

IF(${CAFFE_BUILD})
  TARGET_LINK_LIBRARIES(core ${CAFFE_LIBS})
ENDIF(${CAFFE_BUILD})

ADD_DEPENDENCIES(core operations generated_schemas)
