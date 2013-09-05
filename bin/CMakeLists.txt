SET(bin_files
  hqc
  histhqcdiana
  hqcdiana
  testhqcdiana)

FOREACH(file ${bin_files})
  CONFIGURE_FILE("${file}.in" "${file}-${PVERSION}" @ONLY)
  INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${file}-${PVERSION}" DESTINATION bin)
ENDFOREACH()
