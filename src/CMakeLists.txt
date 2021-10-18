
#########################################################################
# Qt

FIND_PACKAGE(Qt5 COMPONENTS Widgets LinguistTools Sql Network Test Xml REQUIRED)

ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT_NO_KEYWORDS)
LINK_LIBRARIES(Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Xml)

#########################################################################
#########################################################################

SET(PKGS
  libkvcpp>=7.0.0 # kvalobs

  metlibs-tsdata-core>=7.0.0
  metlibs-pets2-qt5>=8.0.0
  metlibs-pumet>=7.0.0
  metlibs-qutilities-qt5>=8.0.0
  metlibs-putools>=8.0.0
  metlibs-milogger>=6.0.0
  metlibs-coserver-qt5>=3.0.1
)

METNO_FIND_PACKAGE("ldap" "" "ldap" "ldap.h")
LIST(APPEND PKGS ${ldap_PC})

METNO_FIND_PACKAGE("sqlite3" "" "sqlite3" "sqlite3.h")
LIST(APPEND PKGS ${sqlite3_PC})

SET(OTHER_LIBS ${ldap_LIB} ${sqlite3_LIB})
SET(OTHER_INC_DIRS ${ldap_INC_DIR} ${sqlite3_INC_DIR})
PKG_CHECK_MODULES(PC REQUIRED ${PKGS})

#########################################################################
# boost

FIND_PACKAGE(Boost COMPONENTS date_time thread REQUIRED)
LINK_LIBRARIES(${Boost_LIBRARIES})

#########################################################################
# includes/defines for all pkg-config libs

INCLUDE_DIRECTORIES(${PC_INCLUDE_DIRS} ${OTHER_INC_DIRS})
ADD_DEFINITIONS(-W -Wall ${PC_CFLAGS_OTHER} ${PC_CFLAGS_OTHER})
LINK_DIRECTORIES(${PC_LIBRARY_DIRS})

# ########################################################################

CONFIGURE_FILE(common/config.h.in config.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

ADD_CUSTOM_TARGET(lupdate)

ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(qtimeseries)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(errorlist)
ADD_SUBDIRECTORY(extremes)
ADD_SUBDIRECTORY(missingobs)
ADD_SUBDIRECTORY(watchrr)
#ADD_SUBDIRECTORY(weather)
ADD_SUBDIRECTORY(rejectedobs)
ADD_SUBDIRECTORY(textdata)
ADD_SUBDIRECTORY(hqc)
