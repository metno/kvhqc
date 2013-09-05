
SET(HQC_SYS_DB ${CMAKE_CURRENT_BINARY_DIR}/hqc_system.db)

ADD_CUSTOM_COMMAND(
   OUTPUT "${HQC_SYS_DB}"
   COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/create_system_db.py "${HQC_SYS_DB}"
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/create_system_db.py
)

ADD_CUSTOM_TARGET(system-db ALL DEPENDS ${HQC_SYS_DB})

CONFIGURE_FILE(hqc_current_version.in hqc_current_version)

INSTALL(FILES
  ${HQC_SYS_DB}
  ${CMAKE_CURRENT_BINARY_DIR}/hqc_current_version
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}")

INSTALL(FILES
  images/hqc.png
  images/kmplot.png
  images/sorttime.png
  images/table.png
  images/watchrr.png
  images/watchweather.png
  images/diana_icon_hqc.png
  images/diana_img_hqc.png
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/images")

INSTALL(FILES
  doc/news.html
  DESTINATION "share/doc/${PROJECT_NAME}-${PVERSION}/html")

FOREACH(df hqcdiana WatchRR WatchRR2 WatchWeather histhqcdiana histWatchRR histWatchWeather)
  SET(dfi metno-${df}.desktop.in)
  SET(dfo metno-${df}-${PVERSION}.desktop)
  CONFIGURE_FILE(${dfi} ${dfo})
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${dfo} DESTINATION "share/applications")
ENDFOREACH()
