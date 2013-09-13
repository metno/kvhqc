
SET(weather_SOURCES
  WeatherDialog.cc
  WeatherStationDialog.cc
  WeatherTableModel.cc
)

QT4_ADD_TRANSLATION(weather_QM
  weather_nb.ts
  weather_de.ts
)

QT4_WRAP_CPP(weather_MOC
  WeatherDialog.hh
  WeatherStationDialog.hh
)

QT4_WRAP_UI(weather_UIH
  weatherdialog.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(weather STATIC
  ${weather_SOURCES} 
  ${weather_MOC}
  ${weather_UIH}
  ${weather_QM}
)

INSTALL(FILES ${weather_QM}
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/lang")
