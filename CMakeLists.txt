PROJECT(kvhqc)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

ENABLE_TESTING()

INCLUDE(MetnoVersions)
METNO_PVERSION(2.14.1)

IF("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  SET(SYSCONFDIR "/etc")
ELSE()
  SET(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc")
ENDIF()

SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTSQL TRUE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QT3SUPPORT TRUE)
FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
LINK_LIBRARIES(${QT_LIBRARIES})

INCLUDE(FindPkgConfig)

PKG_CHECK_MODULES(PC_KVCPP libkvcpp>=2.7.0~rc1 libkvalobs_decodeutility)
IF(NOT PC_KVCPP_FOUND)
  PKG_CHECK_MODULES(PC_KVCPP libkvcpp2>=2.5.3 puTools)
  SET(PC_KVCPP_LIBRARIES ${PC_KVCPP_LIBRARIES} kvalobs2_kvalobs kvalobs2_decodeutility)
  MESSAGE(WARNING "using old kvcpp with static metlibs; this might cause conflicts or crashes")
ENDIF()

PKG_CHECK_MODULES(PC_METLIBSQT REQUIRED qTimeseries qUtilities)
SET(PC_METLIBSQT_LIBRARIES ${PC_METLIBSQT_LIBRARIES} -lGL)

INCLUDE_DIRECTORIES(${PC_KVCPP_INCLUDE_DIRS} ${PC_METLIBSQT_INCLUDE_DIRS})
ADD_DEFINITIONS(-W -Wall ${PC_KVCPP_CFLAGS_OTHER} ${PC_METLIBSQT_CFLAGS_OTHER})

FIND_PACKAGE(Boost COMPONENTS date_time signals thread REQUIRED)
LINK_LIBRARIES(${Boost_DATE_TIME_LIBRARY} ${Boost_SIGNALS_LIBRARY} ${Boost_THREAD_LIBRARY})

CONFIGURE_FILE(include/config.h.in config.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(FILES_TO_TRANSLATE)
FUNCTION(add_translated)
  FOREACH(a ${ARGN})
    SET(FTT ${FTT} "${CMAKE_CURRENT_SOURCE_DIR}/${a}")
  ENDFOREACH()
  SET(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${FTT} PARENT_SCOPE)
ENDFUNCTION()

ADD_SUBDIRECTORY(authentication)
ADD_SUBDIRECTORY(FailInfo)
ADD_SUBDIRECTORY(WatchRR)
ADD_SUBDIRECTORY(WatchRR2)
ADD_SUBDIRECTORY(Weather)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(bin)

SET(etc_files
  etc/kvhqc/diana.setup
  etc/kvhqc/kvalobs.conf
  etc/kvhqc/kvhist.conf
  etc/kvhqc/kvtest.conf
  etc/kvhqc/paramorder
  etc/kvhqc/slimits)
INSTALL(FILES ${etc_files} DESTINATION "${SYSCONFDIR}/kvhqc/${PVERSION}")

CONFIGURE_FILE(share/hqc_current_version.in share/hqc_current_version)
INSTALL(FILES
  share/faildetail.txt
  share/failgroup.txt
  share/VxExplanations.txt
  ${CMAKE_BINARY_DIR}/share/hqc_current_version
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}")

INSTALL(FILES
  share/images/hqc.png
  share/images/kmplot.png
  share/images/sorttime.png
  share/images/table.png
  share/images/watchrr.png
  share/images/watchweather.png
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/images")

FOREACH(df hqcdiana WatchRR WatchRR2 WatchWeather histhqcdiana histWatchRR histWatchWeather)
  SET(dfi share/metno-${df}.desktop.in)
  SET(dfo share/metno-${df}-${PVERSION}.desktop)
  CONFIGURE_FILE(${dfi} ${dfo})
  INSTALL(FILES ${CMAKE_BINARY_DIR}/${dfo} DESTINATION "share/applications")
ENDFOREACH()
