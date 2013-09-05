
SET(auth_HEADERS
  Authenticator.h
  HideApplyBox.hh
  HintWidget.hh
  MiDateTimeEdit.hh
  HqcApplication.hh
  TimeRangeControl.hh
)

SET(auth_SOURCES
  Authenticator.cc
  BusyIndicator.cc
  Functors.hh
  Functors.cc
  HideApplyBox.cc
  HintWidget.cc
  HqcApplication.cc
  HqcDataReinserter.cc
  HqcLogging.cc
  hqc_paths.cc
  hqc_utilities.cc
  identifyUser.cc
  KvMetaDataBuffer.hh
  KvMetaDataBuffer.cc
  MiDateTimeEdit.cc
  Sensor.hh
  Sensor.cc
  TimeRange.cc
  TimeRangeControl.cc
  timeutil.cc
  ToolTipStringListModel.hh
  ToolTipStringListModel.cc
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

ADD_SUBDIRECTORY(test)
