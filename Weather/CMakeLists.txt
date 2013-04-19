
SET(weather_HEADERS
  src/weatherdialog.h
  src/StationSelection.h
  src/centralwidget.h
  src/MultiStationSelection.h
  src/weathertable.h)

SET(weather_SOURCES
  src/StationSelection.cc
  src/MultiStationSelection.cc
  src/cellvalueprovider.cc
  src/weatherdialog.cc
  src/weathertabletooltip.cc
  src/centralwidget.cc
  src/weathertable.cc
  src/dataconsistencyverifier.cc
  src/tnchecktableitem.cc
  src/fdchecktableitem.cc
  src/flagitem.cc
  src/timeobs.cc
  src/weathertableitem.cc
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/authentication
  ${CMAKE_BINARY_DIR}/authentication
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

QT4_WRAP_CPP(weather_HEADERS_MOC ${weather_HEADERS})

ADD_LIBRARY(weather STATIC
  ${weather_SOURCES} 
  ${weather_HEADERS_MOC})

LINK_DIRECTORIES(${PC_KVCPP_LIBRARY_DIRS} ${PC_METLIBSQT_LIBRARY_DIRS} ${PC_LOG4CPP_LIBRARY_DIRS})
ADD_EXECUTABLE(Weather
  src/main.cc)

TARGET_LINK_LIBRARIES(Weather
  weather authentication 
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES}
  -lldap)

SET_TARGET_PROPERTIES(Weather PROPERTIES
  OUTPUT_NAME "WatchWeather-${PVERSION}.bin")

INSTALL(TARGETS Weather DESTINATION bin)
