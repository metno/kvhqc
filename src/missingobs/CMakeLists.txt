
SET(missingobs_SOURCES
  MissingObsQuery.cc
  MissingTableModel.cc
  MissingView.cc
)

QT5_WRAP_CPP(missingobs_MOC
  MissingTableModel.hh
  MissingView.hh
)

SET(missingobs_UIS
  missingvalues.ui
)
QT5_WRAP_UI(missingobs_UIH ${missingobs_UIS})

QT5_CREATE_TRANSLATION(missingobs_QM
  ${missingobs_SOURCES}
  ${missingobs_UIS}

  missingobs_nb.ts
  missingobs_de.ts
)
INSTALL(FILES ${missingobs_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(missingobs STATIC
  ${missingobs_SOURCES}
  ${missingobs_MOC}
  ${missingobs_UIH}
  ${missingobs_QM}
)

ADD_SUBDIRECTORY(test)
