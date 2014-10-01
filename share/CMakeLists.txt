
CONFIGURE_FILE(hqc_current_version.in hqc_current_version)
INSTALL(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/hqc_current_version
  DESTINATION "share/${PROJECT_NAME}/${PVERSION}")

INSTALL(FILES
  images/diana_icon_hqc.png
  images/diana_img_hqc.png

  images/undo.svg
  images/redo.svg
  images/accept.svg
  images/reject.svg
  images/interpolate.svg
  images/dl_columns.svg
  images/dl_columns_add.svg
  images/dl_columns_remove.svg
  images/dl_columns_reset.svg
  images/dl_save_as.svg
  images/hqc_logo.svg
  images/hqc_splash.svg
  images/kv_error.svg
  images/kv_ok.svg
  images/search_red.svg
  images/filter.svg

  DESTINATION "share/${PROJECT_NAME}/${PVERSION}/images")

INSTALL(FILES
  doc/news.html
  DESTINATION "share/doc/${PROJECT_NAME}-${PVERSION}/html")
