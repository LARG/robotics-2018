CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(vision)

INCLUDE(../common.cmake)

FILE(GLOB_RECURSE SRCS ${INTERFACE_DIR}/shared_vision/*.cpp)
SET(LIBPYTHONSWIG ${CMAKE_BINARY_DIR}/../../pythonswig/build-atom/lib_pythonswig_module.so)

QI_CREATE_BIN(vision ${SRCS})
QI_USE_LIB(vision core opencv2_core opencv2_highgui opencv2_flann opencv2_features2d opencv2_calib3d opencv2_imgproc opencv2_objdetect opencv2_ml opencv2_nonfree)
IF(${CAFFE_BUILD})
  TARGET_LINK_LIBRARIES(vision ${LINK_LIBS} ${ALGLIB} ${LIBYAML-CPP} ${LIBYUVIEW} ${LIBPYTHONSWIG} ${LIBCORE} ${LIBFFT} ${LIBFFTW3F} ${LIBPYTHONSWIG} ${CAFFE_LIBS} -lutil ${NAO_HOME}/naoqi/crosstoolchain/atom/boost/lib/libboost_program_options-mt-1_55.so)
ELSE(${CAFFE_BUILD})
  TARGET_LINK_LIBRARIES(vision ${LINK_LIBS} ${ALGLIB} ${LIBYAML-CPP} ${LIBYUVIEW} ${LIBPYTHONSWIG} ${LIBCORE} ${LIBFFT} ${LIBFFTW3F} ${LIBPYTHONSWIG} -lutil ${NAO_HOME}/naoqi/crosstoolchain/atom/boost/lib/libboost_program_options-mt-1_55.so)
ENDIF(${CAFFE_BUILD})
