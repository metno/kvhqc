
SET(weather_HEADERS
  WeatherDialog.hh
  WeatherStationDialog.hh
)

SET(weather_SOURCES
  WeatherDialog.cc
  WeatherStationDialog.cc
  WeatherTableModel.hh
  WeatherTableModel.cc
)

SET(weather_TRANS weather_nb.ts weather_de.ts)
QT4_ADD_TRANSLATION(weather_QM ${weather_TRANS})

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

INSTALL(FILES ${weather_QM}
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")
