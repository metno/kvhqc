
SET(common_gui_SOURCES
  AcceptRejectButtons.cc
  Authenticator.cc
  NeighborHeader.cc
  ObsDelegate.cc
  SensorChooser.cc
  SensorHeader.cc
  TimeHeader.cc
  TimeRangeControl.cc
  ToolInterpolate.cc
)

QT4_WRAP_CPP(common_gui_MOC
  AcceptRejectButtons.hh
  Authenticator.hh
  ObsDelegate.hh
  SensorChooser.hh
  TimeRangeControl.hh
  ToolInterpolate.hh
)

QT4_WRAP_UI(common_gui_UIH
  authenticationdialog.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(common_gui STATIC
  ${common_gui_SOURCES}
  ${common_gui_MOC}
  ${common_gui_UIH}
)
