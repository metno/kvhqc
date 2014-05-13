
SET(hqc_SOURCES
  AutoDataList.cc
  ChecksTableModel.cc
  DataListAddColumn.cc
  DataList.cc
  EditVersionModel.cc
  EditVersionsView.cc
  GetTextData.cc
#  HqcDianaHelper.cc
  HqcMainWindow.cc
  JumpToObservation.cc
  ListDialog.cc
  SimpleCorrections.cc
  StaticDataList.cc
  TimeSeriesAdd.cc
  TimeseriesDialog.cc
  TimeSeriesView.cc
  ViewChanges.cc
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
  EditVersionsView.hh
#  HqcDianaHelper.hh
  HqcMainWindow.hh
  JumpToObservation.hh
  ListDialog.hh
  SimpleCorrections.hh
  StaticDataList.hh
  TimeSeriesAdd.hh
  TimeseriesDialog.h
  TimeSeriesView.hh
  ViewChanges.hh
  UserSettings.hh
)

QT4_WRAP_UI(hqc_UIH
  datalist.ui
  dl_addcolumn.ui
  jumptoobservation.ui
  listdialog.ui
  mainwindow.ui
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
#  weather
#  watchrr
  errorlist
  common
  util
  qtimeseries
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
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")
