
SET(watchrr2_HEADERS
  EditDialog.hh
  MainDialog.hh
  ObsDelegate.hh
  ObsTableModel.hh
  QtKvalobsAccess.hh
  QtKvService.hh
  RedistDialog.hh
  StationDialog.hh
)

SET(watchrr2_SOURCES
  AnalyseFCC.hh
  AnalyseFCC.cc
  AnalyseRR24.hh
  AnalyseRR24.cc
  Code2Text.hh
  Code2Text.cc
  ColumnFactory.hh
  ColumnFactory.cc
  DataColumn.hh
  DataColumn.cc
  EditAccess.hh
  EditAccess.cc
  EditData.hh
  EditData.cc
  EditDataEditor.hh
  EditDataEditor.cc
  EditDialog.hh
  EditDialog.cc
  EditTableModel.hh
  EditTableModel.cc
  EditTimeColumn.hh
  EditTimeColumn.cc
  FlagChange.hh
  FlagChange.cc
  FlagUpdate.hh
  FlagUpdate.cc
  FlagPattern.hh
  FlagPattern.cc
  FlagPatterns.hh
  FlagPatterns.cc
  Helpers.hh
  Helpers.cc
  KvalobsAccess.hh
  KvalobsAccess.cc
  KvalobsData.hh
  KvalobsData.cc
  KvalobsModelAccess.hh
  KvalobsModelAccess.cc
  KvalobsModelData.hh
  KvalobsModelData.cc
  KvBufferedAccess.hh
  KvBufferedAccess.cc
  KvModelAccess.hh
  KvModelAccess.cc
  MainDialog.hh
  MainDialog.cc
  MainTableModel.hh
  MainTableModel.cc
  ModelAccess.hh
  ModelAccess.cc
  ModelColumn.cc
  ModelColumn.hh
  ModelData.hh
  ObsColumn.hh
  ObsColumn.cc
  ObsAccess.hh
  ObsAccess.cc
  ObsData.hh
  ObsDelegate.hh
  ObsDelegate.cc
  ObsSubscription.hh
  ObsTableModel.hh
  ObsTableModel.cc
  ObsUpdate.hh
  QtKvalobsAccess.hh
  QtKvalobsAccess.cc
  QtKvService.hh
  QtKvService.cc
  RedistDialog.hh
  RedistDialog.cc
  RedistTableModel.hh
  RedistTableModel.cc
  RR24Column.hh
  RR24Column.cc
  Sensor.hh
  Sensor.cc
  StationDialog.hh
  StationDialog.cc
  Tasks.hh
  Tasks.cc
  VxColumn.hh
  VxColumn.cc
)

SET(watchrr2_TRANS watchrr2_nb_NO.ts)
QT4_ADD_TRANSLATION(watchrr2_QM ${watchrr2_TRANS})

INCLUDE_DIRECTORIES(
#  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/authentication
#  ${CMAKE_BINARY_DIR}/authentication
#  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

QT4_WRAP_CPP(watchrr2_HEADERS_MOC ${watchrr2_HEADERS})
SET(watchrr2_UIS
    watchrr_edit.ui
    watchrr_main.ui
    watchrr_redist.ui
    watchrr_station.ui
)
QT4_WRAP_UI(watchrr2_UIS_H ${watchrr2_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_translated(${watchrr2_SOURCES})
SET(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} "${watchrr2_UIS_H}" PARENT_SCOPE)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

ADD_LIBRARY(watchrr2
  ${watchrr2_SOURCES} 
  ${watchrr2_HEADERS_MOC}
  ${watchrr2_UIS_H}
  ${watchrr2_QM})

ADD_EXECUTABLE(WatchRR2
  main.cc)

TARGET_LINK_LIBRARIES(WatchRR2
  watchrr2 authentication
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES}
  -lldap)

SET_TARGET_PROPERTIES(WatchRR2 PROPERTIES
  OUTPUT_NAME WatchRR2-${PVERSION})

ADD_SUBDIRECTORY(test)

#INSTALL(TARGETS WatchRR2 DESTINATION bin)
