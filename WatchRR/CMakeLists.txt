
SET(watchRR_HEADERS
  src/StationSelection.h
  src/RRTable.h
  src/RRDialog.h
  src/MultiStationSelection.h)

SET(watchRR_SOURCES
  src/StationSelection.cc
  src/MultiStationSelection.cc
  src/VxKvDataTableItem.cc
  src/RR_24DataTableItem.cc
  src/RRTableToolTip.cc
  src/cellvalueprovider.cc
  src/ControlFlagCell.cc
  src/RRTable.cc
  src/OkCheckTableItem.cc
  src/RRDialog.cc
  src/dataconsistencyverifier.cc
  src/RRTableItem.cc
  src/FDCheckTableItem.cc
#  src/RRCheckTableItem.cc
  src/DayObs.cc
  src/SDDataTableItem.cc
  src/SADataTableItem.cc
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/authentication
  ${CMAKE_BINARY_DIR}/authentication
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

QT4_WRAP_CPP(watchRR_HEADERS_MOC ${watchRR_HEADERS})

ADD_LIBRARY(watchrr
  ${watchRR_SOURCES} 
  ${watchRR_HEADERS_MOC})

ADD_EXECUTABLE(WatchRR
  src/main.cc)

TARGET_LINK_LIBRARIES(WatchRR
  watchrr authentication
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES}
  -lldap)

SET_TARGET_PROPERTIES(WatchRR PROPERTIES
  OUTPUT_NAME WatchRR-${PVERSION})

INSTALL(TARGETS WatchRR DESTINATION bin)
