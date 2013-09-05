
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
  images/diana_icon_hqc.png
  images/diana_img_hqc.png

  images/undo.svg
  images/redo.svg
  images/accept.svg
  images/reject.svg
  images/dl_columns.svg
  images/dl_columns_add.svg
  images/dl_columns_remove.svg
  images/dl_columns_reset.svg
  images/dl_save_as.svg

  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/images")

INSTALL(FILES
  doc/news.html
  DESTINATION "share/doc/${PROJECT_NAME}-${PVERSION}/html")

FOREACH(df hqcdiana histhqcdiana)
  SET(dfi metno-${df}.desktop.in)
  SET(dfo metno-${df}-${PVERSION}.desktop)
  CONFIGURE_FILE(${dfi} ${dfo})
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${dfo} DESTINATION "share/applications")
ENDFOREACH()
