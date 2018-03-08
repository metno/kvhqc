
SET(watchrr_SOURCES
  AnalyseFCC.cc
  AnalyseRR24.cc
  EditDialog.cc
  EditTableModel.cc
  EditTimeColumn.cc
  NeighborCardsModel.cc
  NeighborRR24Model.cc
  RedistDialog.cc
  RedistTableModel.cc
  StationCardModel.cc
  StationDialog.cc
  TaskAccess.cc
  TaskData.cc
  TaskUpdate.cc
  TasksColumn.cc
  WatchRRDialog.cc
  WatchRRTableModel.cc
  WrapperColumn.cc
)

SET(watchrr_UIS
  watchrr_edit.ui
  watchrr_main.ui
  watchrr_redist.ui
  watchrr_station.ui
)

QT5_CREATE_TRANSLATION(watchrr_QM
  ${watchrr_SOURCES}
  ${watchrr_UIS}

  watchrr_de.ts
  watchrr_nb.ts
)
INSTALL(FILES ${watchrr_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

QT5_WRAP_CPP(watchrr_MOC
  EditDialog.hh
  WatchRRDialog.hh
  NeighborCardsModel.hh
  RedistDialog.hh
  StationDialog.hh
)

QT5_WRAP_UI(watchrr_UIH ${watchrr_UIS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(watchrr
  ${watchrr_SOURCES} 
  ${watchrr_MOC}
  ${watchrr_UIH}
  ${watchrr_QM}
)

ADD_SUBDIRECTORY(test)
