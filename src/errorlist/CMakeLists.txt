
SET(errorlist_SOURCES
  ErrorFilter.hh
  ErrorFilter.cc
  ErrorList.cc
  ErrorListModel.cc
  ErrorSearchDialog.cc
)

QT5_WRAP_CPP(errorlist_MOC
  ErrorList.hh
  ErrorListModel.hh
  ErrorSearchDialog.hh
)

SET(errorlist_UIS
  errorlist.ui
  error_search_dialog.ui
)
QT5_WRAP_UI(errorlist_UIH ${errorlist_UIS})

METNO_QT5_CREATE_TRANSLATION(errorlist_QM
  UPDATE_SOURCES
  ${errorlist_SOURCES}
  ${errorlist_UIS}

  TSFILES
  errorlist_nb.ts
  errorlist_de.ts

  UPDATE_TARGET
  lupdate
)

INSTALL(FILES ${errorlist_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(errorlist STATIC
  ${errorlist_SOURCES}
  ${errorlist_MOC}
  ${errorlist_UIH}
  ${errorlist_QM}
)

ADD_SUBDIRECTORY(test)
