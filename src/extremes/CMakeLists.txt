
SET(extremes_SOURCES
  ExtremesFilter.cc
  ExtremesTableModel.cc
  ExtremesView.cc
)

QT5_WRAP_CPP(extremes_MOC
  ExtremesTableModel.hh
  ExtremesView.hh
)

QT5_WRAP_UI(extremes_UIH
  extremevalues.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(extremes STATIC
  ${extremes_SOURCES}
  ${extremes_MOC}
  ${extremes_UIH}
)

ADD_SUBDIRECTORY(test)
