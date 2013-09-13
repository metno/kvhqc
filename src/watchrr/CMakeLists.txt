
SET(watchrr_SOURCES
  EditDialog.cc
  EditTableModel.cc
  MainDialog.cc
  MainTableModel.cc
  NeighborDataModel.cc
  NeighborTableModel.cc
  RedistDialog.cc
  RedistTableModel.cc
  StationDialog.cc
)

QT4_ADD_TRANSLATION(watchrr_QM
  watchrr_de.ts
  watchrr_nb.ts
)

QT4_WRAP_CPP(watchrr_MOC
  EditDialog.hh
  MainDialog.hh
  NeighborDataModel.hh
  RedistDialog.hh
  StationDialog.hh
)

QT4_WRAP_UI(watchrr_UIH
  watchrr_edit.ui
  watchrr_main.ui
  watchrr_redist.ui
  watchrr_station.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_translated(${watchrr_SOURCES})
SET(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} "${watchrr_UIS_H}" PARENT_SCOPE)

ADD_LIBRARY(watchrr
  ${watchrr_SOURCES} 
  ${watchrr_MOC}
  ${watchrr_UIH}
  ${watchrr_QM}
)

INSTALL(FILES ${watchrr_QM}
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")
