
SET(hqc_HEADERS
  ErrorListTableModel.hh
  ../include/DataList.hh
  DataListModel.hh
  ../include/accepttimeseriesdialog.h
  ../include/approvedialog.h
  ../include/dianashowdialog.h
  ../include/discarddialog.h
  ../include/errorlist.h
  ../include/HqcDianaHelper.hh
  ../include/hqcmain.h
  ../include/ItemCheckBox.hh
  ../include/ListDialog.hh
  ../include/rejectdialog.h
  ../include/rejecttable.h
  ../include/rejecttimeseriesdialog.h
  ../include/SimpleCorrections.hh
  ../include/StationSelection.hh
  ../include/textdatadialog.h
  ../include/textdatatable.h
  ../include/TimeseriesDialog.h
  TimeSeriesView.hh
)

SET(hqc_SOURCES
  AnalyseErrors.cc
  AutoColumnView.cc
  ChecksTableModel.cc
  DataList.cc
  DataListModel.cc
  ../include/DataView.hh
  DataView.cc
  ErrorListTableModel.cc
  EditVersionModel.hh
  EditVersionModel.cc
  ParamIdModel.cc
  ListDialog.cc
  StationSelection.cc
  accepttimeseriesdialog.cc
  approvedialog.cc
  dianashowdialog.cc
  discarddialog.cc
  errorlist.cc
  GetTextData.cc
  HqcDianaHelper.cc
  hqcmain.cc
  ItemCheckBox.cc
  qtQTUtil.cc
  rejectdialog.cc
  rejecttable.cc
  rejecttimeseriesdialog.cc
  StationInfoBuffer.cc
  StInfoSysBuffer.cc
  SimpleCorrections.cc
  textdatadialog.cc
  textdatatable.cc
  TimeseriesDialog.cc
  TimeSeriesView.cc
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/authentication
  ${CMAKE_BINARY_DIR}/authentication
  ${CMAKE_SOURCE_DIR}/WatchRR2
  ${CMAKE_SOURCE_DIR}/Weather
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

SET(hqc_TRANS hqc_nb.ts hqc_de.ts)
QT4_ADD_TRANSLATION(hqc_QM ${hqc_TRANS})
INSTALL(FILES ${hqc_QM}
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")

QT4_WRAP_CPP(hqc_HEADERS_MOC ${hqc_HEADERS})
SET(hqc_UIS
  datalist.ui
  dl_addcolumn.ui
  listdialog.ui
  mainwindow.ui
  simplecorrections.ui
  stationselection.ui
  timeseriesview.ui
)
QT4_WRAP_UI(hqc_UIS_H ${hqc_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(hqc_common
  ${hqc_SOURCES}
  ${hqc_HEADERS_MOC}
  ${hqc_UIS_H}
  ${hqc_QM}
)

LINK_DIRECTORIES(${PC_KVCPP_LIBRARY_DIRS} ${PC_METLIBSQT_LIBRARY_DIRS} ${PC_LOG4CPP_LIBRARY_DIRS})
ADD_EXECUTABLE(hqc
  hqc4.cc
)

SET_TARGET_PROPERTIES(hqc PROPERTIES
  OUTPUT_NAME "hqc-${PVERSION}.bin")

TARGET_LINK_LIBRARIES(hqc
  hqc_common watchrr2 weather authentication
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES} ${PC_LOG4CPP_LIBRARIES}
  -lldap
)

INSTALL(TARGETS hqc DESTINATION bin)

ADD_EXECUTABLE(hqc_offline
  hqc_offline.cc
)

SET_TARGET_PROPERTIES(hqc_offline PROPERTIES
  OUTPUT_NAME "hqc-${PVERSION}-offline")

TARGET_LINK_LIBRARIES(hqc_offline
  hqc_common watchrr2 weather authentication watchrr2test
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES} ${PC_LOG4CPP_LIBRARIES}
)

INSTALL(TARGETS hqc_offline DESTINATION bin)
