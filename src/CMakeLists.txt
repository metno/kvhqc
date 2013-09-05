
SET(hqc_HEADERS
  ErrorListTableModel.hh
  ../include/ChecksTableModel.hh
  ../include/DataList.hh
  ../include/DataListAddColumn.hh
  ../include/DataListModel.hh
  ../include/dianashowdialog.h
  ../include/EditVersionModel.hh
  ../include/errorlist.h
  ../include/ExtremesView.hh
  ../include/HqcDianaHelper.hh
  ../include/hqcmain.h
  ../include/ItemCheckBox.hh
  ../include/ListDialog.hh
  ../include/rejectdialog.h
  ../include/rejecttable.h
  ../include/SimpleCorrections.hh
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
  DataListAddColumn.cc
  DataListModel.cc
  ../include/DataView.hh
  DataView.cc
  ErrorListTableModel.cc
  EditVersionModel.cc
  ExtremesTableModel.cc
  ExtremesView.cc
  FindAllParameters.cc
  FindExtremeValues.cc
  ParamIdModel.cc
  ListDialog.cc
  dianashowdialog.cc
  errorlist.cc
  GetTextData.cc
  HqcDianaHelper.cc
  hqcmain.cc
  ItemCheckBox.cc
  qtQTUtil.cc
  rejectdialog.cc
  rejecttable.cc
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
  extremevalues.ui
  listdialog.ui
  mainwindow.ui
  simplecorrections.ui
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
