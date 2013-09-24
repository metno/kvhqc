
#########################################################################
# Qt

SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTSQL TRUE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QT3SUPPORT TRUE)
SET(QT_USE_QTXML TRUE)
FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS)
LINK_LIBRARIES(${QT_LIBRARIES})

#########################################################################
#########################################################################
# kvalobs

PKG_CHECK_MODULES(PC_KVCPP libkvcpp>=2.7.0~rc1 libkvalobs_decodeutility)
IF(NOT PC_KVCPP_FOUND)
  PKG_CHECK_MODULES(PC_KVCPP2 libkvcpp2>=2.5.3 puTools libpqxx)
  IF(NOT PC_KVCPP2_FOUND)
    ERROR("could not find required kvcpp libs")
  ENDIF()
  MESSAGE(WARNING "using old kvcpp with static metlibs; this might cause conflicts or crashes")
  SET(PC_KVCPP_LIBRARIES ${PC_KVCPP2_LIBRARIES} kvalobs2_kvalobs kvalobs2_decodeutility kvalobs2_dnmithread CACHE INTERNAL "")
  SET(PC_KVCPP_INCLUDE_DIRS ${PC_KVCPP2_INCLUDE_DIRS} CACHE INTERNAL "")
  SET(PC_KVCPP_CFLAGS_OTHER ${PC_KVCPP2_CFLAGS_OTHER})
ENDIF()

#########################################################################
# metlibs

PKG_CHECK_MODULES(PC_METLIBSQT REQUIRED qTimeseries qUtilities)
SET(PC_METLIBSQT_LIBRARIES ${PC_METLIBSQT_LIBRARIES} -lGL)
IF("${PC_METLIBSQT_qUtilities_VERSION}" VERSION_LESS "4.9.4")
  ADD_DEFINITIONS(-DMETLIBS_BEFORE_4_9_5)
ENDIF()

#########################################################################
# log4cpp

PKG_CHECK_MODULES(PC_LOG4CPP REQUIRED log4cpp)

#########################################################################
# boost

FIND_PACKAGE(Boost COMPONENTS date_time signals thread REQUIRED)
LINK_LIBRARIES(${Boost_DATE_TIME_LIBRARY} ${Boost_SIGNALS_LIBRARY} ${Boost_THREAD_LIBRARY})

#########################################################################
# includes/defines for all pkg-config libs

INCLUDE_DIRECTORIES(${PC_KVCPP_INCLUDE_DIRS} ${PC_METLIBSQT_INCLUDE_DIRS})
ADD_DEFINITIONS(-W -Wall ${PC_KVCPP_CFLAGS_OTHER} ${PC_METLIBSQT_CFLAGS_OTHER} ${PC_LOG4CPP_CFLAGS_OTHER})
LINK_DIRECTORIES(${PC_KVCPP_LIBRARY_DIRS} ${PC_METLIBSQT_LIBRARY_DIRS} ${PC_LOG4CPP_LIBRARY_DIRS})

#########################################################################
#########################################################################
# google-test

FIND_PACKAGE(GTest QUIET)
IF(NOT GTEST_FOUND)
  MESSAGE("apparently no compiled GTest library, trying to build it")
  FIND_FILE(GTEST_DIR src/gtest-all.cc /usr/src/gtest /usr/local/src/gtest)
  IF(NOT GTEST_DIR)
    MESSAGE(FATAL_ERROR "could not find gtest-all.cc")
  ENDIF()
  GET_FILENAME_COMPONENT(GTEST_DIR ${GTEST_DIR} PATH)
  GET_FILENAME_COMPONENT(GTEST_DIR ${GTEST_DIR} PATH)
  ADD_SUBDIRECTORY(${GTEST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
  SET(GTEST_LIBRARY gtest)
  SET(GTEST_MAIN_LIBRARY gtest_main)
ENDIF()

# ########################################################################

CONFIGURE_FILE(common/config.h.in config.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(watchrr)
ADD_SUBDIRECTORY(weather)
ADD_SUBDIRECTORY(hqc)
