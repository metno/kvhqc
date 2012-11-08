PROJECT(kvhqc)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(MetnoVersions)
METNO_PVERSION(2.12.0~rc1)

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
PKG_CHECK_MODULES(PKGCONFIG REQUIRED libkvcpp2>=2.5.3 puTools qTimeseries qUtilities)
INCLUDE_DIRECTORIES(${PKGCONFIG_INCLUDE_DIRS})
ADD_DEFINITIONS(-W -Wall ${PKGCONFIG_CFLAGS_OTHER})

FIND_PACKAGE(Boost COMPONENTS date_time REQUIRED)
LINK_LIBRARIES(${Boost_DATE_TIME_LIBRARY})

CONFIGURE_FILE(include/config.h.in config.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(authentication)
ADD_SUBDIRECTORY(FailInfo)
ADD_SUBDIRECTORY(WatchRR)
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

INSTALL(FILES
  share/faildetail.txt
  share/failgroup.txt
  share/VxExplanations.txt
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}")

INSTALL(FILES
  share/images/hqc.png
  share/images/kmplot.png
  share/images/sorttime.png
  share/images/table.png
  share/images/watchrr.png
  share/images/watchweather.png
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/images")

FOREACH(df hqcdiana WatchRR WatchWeather histhqcdiana histWatchRR histWatchWeather)
  SET(dfi share/metno-${df}.desktop.in)
  SET(dfo share/metno-${df}-${PVERSION}.desktop)
  CONFIGURE_FILE(${dfi} ${dfo})
  INSTALL(FILES ${CMAKE_BINARY_DIR}/${dfo} DESTINATION "share/applications")
ENDFOREACH()
