
SET(missingobs_SOURCES
  MissingObsQuery.cc
  MissingTableModel.cc
  MissingView.cc
)

QT5_WRAP_CPP(missingobs_MOC
  MissingTableModel.hh
  MissingView.hh
)

QT5_WRAP_UI(missingobs_UIH
  missingvalues.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(missingobs STATIC
  ${missingobs_SOURCES}
  ${missingobs_MOC}
  ${missingobs_UIH}
)

ADD_SUBDIRECTORY(test)
