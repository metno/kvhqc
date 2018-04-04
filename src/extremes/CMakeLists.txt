
SET(extremes_SOURCES
  ExtremesFilter.cc
  ExtremesTableModel.cc
  ExtremesView.cc
)

QT5_WRAP_CPP(extremes_MOC
  ExtremesTableModel.hh
  ExtremesView.hh
)

SET(extremes_UIS
  extremevalues.ui
)
QT5_WRAP_UI(extremes_UIH ${extremes_UIS})

METNO_QT5_CREATE_TRANSLATION(extremes_QM
  UPDATE_SOURCES
  ${extremes_SOURCES}
  ${extremes_UIS}

  TSFILES
  extremes_nb.ts
  extremes_de.ts

  UPDATE_TARGET
  lupdate
)

INSTALL(FILES ${extremes_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(extremes STATIC
  ${extremes_SOURCES}
  ${extremes_MOC}
  ${extremes_UIH}
  ${extremes_QM}
)

ADD_SUBDIRECTORY(test)
