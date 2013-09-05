
SET(weather_HEADERS
  WeatherDialog.hh
)

SET(weather_SOURCES
  WeatherDialog.cc
  WeatherStationDialog.hh
  WeatherStationDialog.cc
  WeatherTableModel.hh
  WeatherTableModel.cc
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/authentication
  ${CMAKE_SOURCE_DIR}/WatchRR2
  ${CMAKE_BINARY_DIR}/authentication
)

QT4_WRAP_CPP(weather_HEADERS_MOC ${weather_HEADERS})
SET(weather_UIS
    weatherdialog.ui
)
QT4_WRAP_UI(weather_UIS_H ${weather_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(weather STATIC
  ${weather_SOURCES} 
  ${weather_HEADERS_MOC}
  ${weather_UIS_H}
  ${weather_QM}
)

#LINK_DIRECTORIES(${PC_KVCPP_LIBRARY_DIRS} ${PC_METLIBSQT_LIBRARY_DIRS} ${PC_LOG4CPP_LIBRARY_DIRS})
#ADD_EXECUTABLE(Weather
#  src/main.cc)

#TARGET_LINK_LIBRARIES(Weather
#  weather authentication 
#  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES}
#  -lldap)

#SET_TARGET_PROPERTIES(Weather PROPERTIES
#  OUTPUT_NAME "WatchWeather-${PVERSION}.bin")

#INSTALL(TARGETS Weather DESTINATION bin)
