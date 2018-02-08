
SET(hqc_SOURCES
  AbstractDataView.cc
  AutoDataList.cc
  ChecksTableModel.cc
  DataHistoryTableModel.cc
  DataListAddColumn.cc
  DynamicDataView.cc
  DataList.cc
  EditVersionModel.cc
  EditVersionsView.cc
  HqcAppWindow.cc
#  HqcDianaHelper.cc
  JumpToObservation.cc
  NavigationHistory.cc
  SearchWindow.cc
  SimpleCorrections.cc
  StationDataList.cc
  TimeSeriesAdd.cc
  TimeSeriesView.cc
  TimespanDataList.cc
  ViewChanges.cc
  UserSettings.cc
)

QT5_ADD_TRANSLATION(hqc_QM
  hqc_nb.ts
  hqc_de.ts
)

QT5_WRAP_CPP(hqc_MOC
  AbstractDataView.hh
  AutoDataList.hh
  DataHistoryTableModel.hh
  DataListAddColumn.hh
  DataList.hh
  DynamicDataView.hh
  EditVersionModel.hh
  EditVersionsView.hh
  HqcAppWindow.hh
#  HqcDianaHelper.hh
  JumpToObservation.hh
  NavigationHistory.hh
  SearchWindow.hh
  SimpleCorrections.hh
  StationDataList.hh
  TimeSeriesAdd.hh
  TimeSeriesView.hh
  TimespanDataList.hh
  UserSettings.hh
)

QT5_WRAP_UI(hqc_UIH
  appwindow.ui
  datalist.ui
  dl_addcolumn.ui
  jumptoobservation.ui
  navigationhistory.ui
  singleobservation.ui
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
#  weather
  watchrr
  errorlist
  extremes
  missingobs
  rejectedobs
  textdata
  common
  util
  qtimeseries
  ${PC_LIBRARIES}
  ${OTHER_LIBS}
)

TARGET_LINK_LIBRARIES(hqc
  hqc_common
  ${HQC_COMMON_LIBS}
)

INSTALL(TARGETS hqc DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

## ADD_EXECUTABLE(hqc_offline
##   hqc_offline.cc
## )
## 
## SET_TARGET_PROPERTIES(hqc_offline PROPERTIES
##   OUTPUT_NAME "hqc-${PVERSION}-offline")
## 
## TARGET_LINK_LIBRARIES(hqc_offline
##   hqc_common
##   test_common
##   ${HQC_COMMON_LIBS}
## )

INSTALL(FILES ${hqc_QM}
  DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

ADD_SUBDIRECTORY(test)
