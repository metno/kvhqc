
SET(hqc_HEADERS
  ../include/approvedialog.h
  ../include/dianashowdialog.h
  ../include/HqcDianaHelper.hh
  ../include/KvalobsDataView.h
  ../include/rejecttimeseriesdialog.h
  ../include/accepttimeseriesdialog.h
  ../include/ItemCheckBox.hh
  ../include/KvalobsDataModel.h
  ../include/ListDialog.hh
  ../include/textdatadialog.h
  ../include/StationSelection.hh
  ../include/TimeseriesDialog.h
  ../include/discarddialog.h
  ../include/KvalobsDataDelegate.h
  ../include/hqcmain.h
  ../include/textdatatable.h
  ../include/rejecttable.h
  ../include/rejectdialog.h
  ../include/errorlist.h)

SET(hqc_SOURCES
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
  missingtable.cc
  qtQTUtil.cc
  rejectdialog.cc
  rejecttable.cc
  rejecttimeseriesdialog.cc
  StInfoSysBuffer.cc
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

QT4_WRAP_CPP(hqc_HEADERS_MOC ${hqc_HEADERS})
SET(hqc_UIS
  listdialog.ui
  mainwindow.ui
  stationselection.ui
)
QT4_WRAP_UI(hqc_UIS_H ${hqc_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(hqc
  ${hqc_SOURCES}
  ${hqc_HEADERS_MOC}
  ${hqc_UIS_H})

SET_TARGET_PROPERTIES(hqc PROPERTIES
  OUTPUT_NAME "hqc-${PVERSION}.bin")

TARGET_LINK_LIBRARIES(hqc
  watchrr2 authentication weather failinfo
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES}
  -lldap
)

INSTALL(TARGETS hqc DESTINATION bin)
