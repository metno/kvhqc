
SET(weather_SOURCES
  WeatherDialog.cc
  WeatherStationDialog.cc
  WeatherTableModel.cc
)

SET(weather_UIS
  weatherdialog.ui
)

QT5_CREATE_TRANSLATION(weather_QM
  ${weather_SOURCES}
  ${weather_UIS}

  weather_nb.ts
  weather_de.ts
)
INSTALL(FILES ${weather_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

QT5_WRAP_CPP(weather_MOC
  WeatherDialog.hh
  WeatherStationDialog.hh
)

QT5_WRAP_UI(weather_UIH ${weather_UIS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(weather STATIC
  ${weather_SOURCES} 
  ${weather_MOC}
  ${weather_UIH}
  ${weather_QM}
)
