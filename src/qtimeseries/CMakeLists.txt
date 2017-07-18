
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
)

SET(qtimeseries_SOURCES
  TSdrawarea.cc
  Linetype.cc
  PlotOptions.cc
  Colour.cc
  TSglwidget.cc
)

QT5_WRAP_CPP(qtimeseries_MOC
  TSglwidget.h
)

ADD_LIBRARY(qtimeseries STATIC
  ${qtimeseries_SOURCES}
  ${qtimeseries_MOC}
  ${qtimeseries_UIH}
)
