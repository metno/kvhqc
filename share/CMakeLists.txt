
CONFIGURE_FILE(hqc_current_version.in hqc_current_version)
INSTALL(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/hqc_current_version
  DESTINATION "${HQC_INSTALL_SHAREDIR}")

INSTALL(FILES
  images/diana_icon_hqc.png
  images/diana_img_hqc.png

  images/accept.svg
  images/busy_animation.svg
  images/dl_columns.svg
  images/dl_columns_add.svg
  images/dl_columns_remove.svg
  images/dl_columns_reset.svg
  images/dl_save_as.svg
  images/earlier.svg
  images/editor.svg
  images/error.svg
  images/extremes.svg
  images/filter.svg
  images/forget.svg
  images/history.svg
  images/hqc_logo.svg
  images/hqc_splash.svg
  images/interpolate.svg
  images/jump_to_obs.svg
  images/kv_error.svg
  images/kv_ok.svg
  images/later.svg
  images/missing.svg
  images/redo.svg
  images/reject.svg
  images/rejectedobs.svg
  images/search_red.svg
  images/singleobservation.svg
  images/textdata.svg
  images/timeseries.svg
  images/undo.svg
  images/watchrr.svg
  images/weatherstation.svg

  DESTINATION "${HQC_INSTALL_SHAREDIR}/images")

INSTALL(FILES
  doc/news.html
  doc/intro.html
  DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/doc/${PROJECT_NAME}-${PVERSION}/html")
