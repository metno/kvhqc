
SET(hqc_HEADERS
  ErrorListTableModel.hh
  DataList.hh
  DataListModel.hh
  ../include/accepttimeseriesdialog.h
  ../include/approvedialog.h
  ../include/dianashowdialog.h
  ../include/discarddialog.h
  ../include/errorlist.h
  ../include/HqcDianaHelper.hh
  ../include/hqcmain.h
  ../include/ItemCheckBox.hh
  ../include/KvalobsDataDelegate.h
  ../include/KvalobsDataModel.h
  ../include/KvalobsDataView.h
  ../include/ListDialog.hh
  ../include/rejectdialog.h
  ../include/rejecttable.h
  ../include/rejecttimeseriesdialog.h
  ../include/SimpleCorrections.hh
  ../include/StationSelection.hh
  ../include/textdatadialog.h
  ../include/textdatatable.h
  ../include/TimeseriesDialog.h
)

SET(hqc_SOURCES
  AnalyseErrors.cc
  DataList.cc
  DataListModel.cc
  ErrorListTableModel.cc
  ParamIdModel.cc
  ListDialog.cc
  StationSelection.cc
  accepttimeseriesdialog.cc
  approvedialog.cc
  dianashowdialog.cc
  discarddialog.cc
  errorlist.cc
  GetData.cc
  GetTextData.cc
  hqc4.cc
  HqcDianaHelper.cc
  hqcmain.cc
  ItemCheckBox.cc
  KvalobsData.cc
  KvalobsDataDelegate.cc
  KvalobsDataModel.cc
  KvalobsDataView.cc
  qtQTUtil.cc
  rejectdialog.cc
  rejecttable.cc
  rejecttimeseriesdialog.cc
  StInfoSysBuffer.cc
  SimpleCorrections.cc
  textdatadialog.cc
  textdatatable.cc
  TimeseriesDialog.cc
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/authentication
  ${CMAKE_BINARY_DIR}/authentication
  ${CMAKE_SOURCE_DIR}/FailInfo/src
  ${CMAKE_BINARY_DIR}/FailInfo
  ${CMAKE_SOURCE_DIR}/WatchRR2
  ${CMAKE_SOURCE_DIR}/Weather/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

SET(hqc_TRANS hqc_nb.ts hqc_de.ts)
QT4_ADD_TRANSLATION(hqc_QM ${hqc_TRANS})

QT4_WRAP_CPP(hqc_HEADERS_MOC ${hqc_HEADERS})
SET(hqc_UIS
  listdialog.ui
  mainwindow.ui
  simplecorrections.ui
  stationselection.ui
)
QT4_WRAP_UI(hqc_UIS_H ${hqc_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

LINK_DIRECTORIES(${PC_KVCPP_LIBRARY_DIRS} ${PC_METLIBSQT_LIBRARY_DIRS} ${PC_LOG4CPP_LIBRARY_DIRS})
ADD_EXECUTABLE(hqc
  ${hqc_SOURCES}
  ${hqc_HEADERS_MOC}
  ${hqc_UIS_H}
  ${hqc_QM}
)

SET_TARGET_PROPERTIES(hqc PROPERTIES
  OUTPUT_NAME "hqc-${PVERSION}.bin")

TARGET_LINK_LIBRARIES(hqc
  watchrr2 weather failinfo authentication
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES} ${PC_LOG4CPP_LIBRARIES}
  -lldap
)

INSTALL(TARGETS hqc DESTINATION bin)

INSTALL(FILES ${hqc_QM}
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")
