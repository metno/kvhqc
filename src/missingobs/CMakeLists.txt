
SET(missingobs_SOURCES
  FindMissingValues.cc
  MissingView.cc
  MissingTableModel.cc
)

QT4_WRAP_CPP(missingobs_MOC
  MissingView.hh
)

QT4_WRAP_UI(missingobs_UIH
  missingvalues.ui
)

ADD_LIBRARY(missingobs STATIC
  ${missingobs_SOURCES}
  ${missingobs_MOC}
  ${missingobs_UIH}
)

#ADD_SUBDIRECTORY(test)
