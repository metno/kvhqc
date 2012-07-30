
SET(hqc_HEADERS
  ../include/approvedialog.h
  ../include/dianashowdialog.h
  ../include/KvalobsDataView.h
  ../include/rejecttimeseriesdialog.h
  ../include/accepttimeseriesdialog.h
  ../include/parameterdialog.h
  ../include/KvalobsDataModel.h
  ../include/ListDialog.h
  ../include/textdatadialog.h
  ../include/TimeseriesDialog.h
  ../include/discarddialog.h
  ../include/KvalobsDataDelegate.h
  ../include/hqcmain.h
  ../include/textdatatable.h
  ../include/rejecttable.h
  ../include/rejectdialog.h
  ../include/ClockDialog.h
  ../include/errorlist.h)

SET(hqc_SOURCES
  accepttimeseriesdialog.cc
  approvedialog.cc
  ClockDialog.cc
  connect2stinfosys.cc
  dianashowdialog.cc
  discarddialog.cc
  errorlist.cc
  GetData.cc
  GetTextData.cc
  hqc4.cc
  hqcmain.cc
  KvalobsData.cc
  KvalobsDataDelegate.cc
  KvalobsDataModel.cc
  KvalobsDataView.cc
  ListDialog.cc
  missingtable.cc
  parameterdialog.cc
  qtQTUtil.cc
  rejectdialog.cc
  rejecttable.cc
  rejecttimeseriesdialog.cc
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
  ${CMAKE_SOURCE_DIR}/WatchRR/src
  ${CMAKE_SOURCE_DIR}/Weather/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

QT4_WRAP_CPP(hqc_HEADERS_MOC ${hqc_HEADERS})

ADD_EXECUTABLE(hqc
  ${hqc_SOURCES} 
  ${hqc_HEADERS_MOC})

SET_TARGET_PROPERTIES(hqc PROPERTIES
  OUTPUT_NAME hqc-${PVERSION})

TARGET_LINK_LIBRARIES(hqc
  watchrr authentication weather failinfo
  ${PKGCONFIG_LIBRARIES}
  -lkvalobs2_kvalobs -lboost_thread -lkvalobs2_decodeutility -lGL -lQtNetwork -lldap
)

INSTALL(TARGETS hqc DESTINATION bin)
