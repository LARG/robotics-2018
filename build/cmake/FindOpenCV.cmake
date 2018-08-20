FIND_PATH(OPENCV2_INCLUDE_PATH opencv.hpp
	# installation selected by user
	$ENV{OPENCV_HOME}/include
	# system placed in /usr/local/include
	/usr/local/include/opencv2
	# system placed in /usr/include
	/usr/include/opencv2
)

FIND_PATH(OPENCV2_LIB_PATH libopencv_core.so
  $ENV{OPENCV_HOME}/lib
  /usr/lib
  /usr/lib/x86_64-linux-gnu
)

IF(OPENCV2_INCLUDE_PATH)
  IF(OPENCV2_LIB_PATH)
    MESSAGE(STATUS "Looking for OpenCV2.4 or greater - found")
    MESSAGE(STATUS "OpenCV2.4 include path: ${OPENCV2_INCLUDE_PATH}")
    SET(OPENCV2_FOUND 1)
  ELSE(OPENCV2_LIB_PATH)
    MESSAGE(STATUS "Looking for OpenCV2.4 libs - not found")
  ENDIF(OPENCV2_LIB_PATH)
ELSE(OPENCV2_INCLUDE_PATH)
  MESSAGE(STATUS "Looking for OpenCV2.4 headers - not found")
  SET(OPENCV2_FOUND 0)
ENDIF(OPENCV2_INCLUDE_PATH)
IF(OPENCV2_FOUND)
  FILE(GLOB OPENCV2_LIBS "${OPENCV2_LIB_PATH}/libopencv_*.so")
  SET(OPENCV2_INCLUDES ${OPENCV2_INCLUDE_PATH})
ENDIF(OPENCV2_FOUND)

