
SET(extremes_SOURCES
  FindMissingValues.cc
  ExtremesTableModel.cc
  ExtremesView.cc
)

QT4_WRAP_CPP(extremes_MOC
  ExtremesView.hh
)

QT4_WRAP_UI(extremes_UIH
  extremevalues.ui
)

ADD_LIBRARY(extremes STATIC
  ${extremes_SOURCES}
  ${extremes_MOC}
  ${extremes_UIH}
)

#ADD_SUBDIRECTORY(test)
