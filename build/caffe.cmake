#cmake_minimum_required(VERSION 2.8)
#set(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(-DCPU_ONLY) # Disable GPU for nao caffe builds
ADD_DEFINITIONS(-DUSE_OPENCV) # Allow OpenCV support in caffe
SET(CAFFE_INCLUDE ${NAO_HOME}/lib/caffe/include ${NAO_HOME}/lib/caffe/build/src)
IF(TOOL_BUILD)
  SET(CAFFE_LIBS -L${NAO_HOME}/lib/caffe/build/lib -lhdf5_serial_hl -lhdf5_serial)
ELSE(TOOL_BUILD)
  SET(CAFFE_LIBS -L${NAO_HOME}/naoqi/link_libs -lhdf5_hl -lhdf5)
ENDIF(TOOL_BUILD)
SET(CAFFE_LIBS ${CAFFE_LIBS}
  -lcaffe
  -lglog
  -lhdf5_cpp
  -lhdf5_hl_cpp
  -lcblas
  -lgflags
  -lprotobuf
  -lleveldb
  -lunwind
  -latlas
  -lcblas
  -llmdb
  -lsnappy
  -llzma
)

