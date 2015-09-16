
#########################################################################
# Qt

SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTSQL TRUE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QT3SUPPORT TRUE)
SET(QT_USE_QTXML TRUE)
FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS -DBOOST_SIGNALS_NO_DEPRECATION_WARNING)
LINK_LIBRARIES(${QT_LIBRARIES})

#########################################################################
#########################################################################

### FIXME this is not working properly, maybe because CMake is caching something
### see http://www.itk.org/Wiki/CMake:How_To_Find_Libraries

# kvalobs
PKG_CHECK_MODULES(PC_KVCPP REQUIRED libkvcpp>=2.7.2 libkvalobs_decodeutility)

# metlibs
PKG_CHECK_MODULES(PC_METLIBS REQUIRED
  tsData>=5.0.0
  pets2>=5.0.0
  puMet>=5.0.0
  qUtilities>=5.0.0
  puTools>=5.0.0
  miLogger>=5.0.0
  coserver>=1.0.1
)

# log4cpp
PKG_CHECK_MODULES(PC_LOG4CPP REQUIRED log4cpp)

#########################################################################
# boost

FIND_PACKAGE(Boost COMPONENTS date_time signals thread REQUIRED)
LINK_LIBRARIES(${Boost_DATE_TIME_LIBRARY} ${Boost_SIGNALS_LIBRARY} ${Boost_THREAD_LIBRARY})

#########################################################################
# includes/defines for all pkg-config libs

INCLUDE_DIRECTORIES(${PC_KVCPP_INCLUDE_DIRS} ${PC_METLIBS_INCLUDE_DIRS})
ADD_DEFINITIONS(-W -Wall ${PC_KVCPP_CFLAGS_OTHER} ${PC_METLIBS_CFLAGS_OTHER} ${PC_LOG4CPP_CFLAGS_OTHER})
LINK_DIRECTORIES(${PC_KVCPP_LIBRARY_DIRS} ${PC_METLIBS_LIBRARY_DIRS} ${PC_LOG4CPP_LIBRARY_DIRS})

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
ADD_SUBDIRECTORY(qtimeseries)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(watchrr)
ADD_SUBDIRECTORY(weather)
ADD_SUBDIRECTORY(hqc)
