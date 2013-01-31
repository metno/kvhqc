
SET(auth_HEADERS
  Authenticator.h
  HideApplyBox.hh
  HintWidget.hh
  MiDateTimeEdit.hh
)

SET(auth_SOURCES
  Authenticator.cc
  BusyIndicator.cc
  debug.cc
  HideApplyBox.cc
  HintWidget.cc
  HqcDataReinserter.cc
  hqc_paths.cc
  hqc_utilities.cc
  identifyUser.cc
  MiDateTimeEdit.cc
  TimeRange.cc
  timeutil.cc
)

QT4_WRAP_CPP(auth_HEADERS_MOC ${auth_HEADERS})
SET(auth_UIS
    authenticationdialog.ui
    hideapplybox.ui
)
QT4_WRAP_UI(auth_UIS_H ${auth_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(authentication STATIC
  ${auth_SOURCES}
  ${auth_HEADERS_MOC}
  ${auth_UIS_H})
