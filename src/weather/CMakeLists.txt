
SET(weather_SOURCES
  WeatherDialog.cc
  WeatherStationDialog.cc
  WeatherTableModel.cc
)

SET(weather_UIS
  weatherdialog.ui
)

METNO_QT5_CREATE_TRANSLATION(weather_QM
  UPDATE_SOURCES
  ${weather_SOURCES}
  ${weather_UIS}

  TSFILES
  weather_nb.ts
  weather_de.ts

  UPDATE_TARGET
  lupdate
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
