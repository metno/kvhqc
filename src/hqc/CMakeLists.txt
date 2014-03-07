
SET(hqc_SOURCES
  AutoDataList.cc
  ChecksTableModel.cc
  DataListAddColumn.cc
  DataList.cc
  EditVersionModel.cc
  ErrorList.cc
  ErrorListModel.cc
  ExtremesTableModel.cc
  ExtremesView.cc
  GetTextData.cc
  HqcDianaHelper.cc
  HqcMainWindow.cc
  JumpToObservation.cc
  ListDialog.cc
  RejectedObs.cc
  RejectedObsDialog.cc
  SimpleCorrections.cc
  StaticDataList.cc
  TextdataDialog.cc
  TextdataTable.cc
  TimeSeriesAdd.cc
  TimeseriesDialog.cc
  TimeSeriesView.cc
  ViewChanges.cc
  MissingView.cc
  MissingTableModel.cc
  UserSettings.cc
)

QT4_ADD_TRANSLATION(hqc_QM
  hqc_nb.ts
  hqc_de.ts
)

QT4_WRAP_CPP(hqc_MOC
  AutoDataList.hh
  ChecksTableModel.hh
  DataListAddColumn.hh
  DataList.hh
  EditVersionModel.hh
  ErrorList.hh
  ErrorListModel.hh
  ExtremesView.hh
  HqcDianaHelper.hh
  HqcMainWindow.hh
  JumpToObservation.hh
  ListDialog.hh
  MissingView.hh
  RejectedObs.hh
  RejectedObsDialog.hh
  SimpleCorrections.hh
  StaticDataList.hh
  TextdataDialog.hh
  TextdataTable.hh
  TimeSeriesAdd.hh
  TimeseriesDialog.h
  TimeSeriesView.hh
  ViewChanges.hh
  UserSettings.hh
  missing/MissingView.hh
  missing/MissingSelectorWidget.hh
  missing/MissingObservationWidget.hh
  missing/StationOrderedMissingDataModel.hh
)

QT4_WRAP_UI(hqc_UIH
  datalist.ui
  dl_addcolumn.ui
  extremevalues.ui
  jumptoobservation.ui
  listdialog.ui
  mainwindow.ui
  missingvalues.ui
  simplecorrections.ui
  timeseriesview.ui
  ts_add.ui
  ts_remove.ui
  user_settings.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(hqc_common
  ${hqc_SOURCES}
  ${hqc_MOC}
  ${hqc_UIH}
  ${hqc_QM}
)

ADD_EXECUTABLE(hqc
  hqc4.cc
)

SET_TARGET_PROPERTIES(hqc PROPERTIES
  OUTPUT_NAME "hqc-${PVERSION}.bin")

SET(HQC_COMMON_LIBS 
  weather
  watchrr
  common
  qtimeseries
  util_gui
  common_gui
  util
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES} 
  ${PC_LOG4CPP_LIBRARIES}
)

TARGET_LINK_LIBRARIES(hqc
  hqc_common
  ${HQC_COMMON_LIBS}
  -lldap
)

INSTALL(TARGETS hqc DESTINATION bin)

ADD_EXECUTABLE(hqc_offline
  hqc_offline.cc
)

SET_TARGET_PROPERTIES(hqc_offline PROPERTIES
  OUTPUT_NAME "hqc-${PVERSION}-offline")

TARGET_LINK_LIBRARIES(hqc_offline
  hqc_common
  test_common
  ${HQC_COMMON_LIBS}
)

INSTALL(FILES ${hqc_QM}
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")
