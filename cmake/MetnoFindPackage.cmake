
FUNCTION(METNO_FIND_PACKAGE name
    pkg_pc
    pkg_libname
    pkg_hdr
    )

  UNSET(p_pc_FOUND CACHE)

  IF(pkg_pc)
    PKG_CHECK_MODULES(p_pc QUIET "${pkg_pc}")
  ENDIF()
  IF(p_pc_FOUND)
    MESSAGE(STATUS "Found ${name}: pkg-config '${pkg_pc}'")
    SET(${name}_PC ${pkg_pc} PARENT_SCOPE)
  ELSE()
    UNSET (p_HEADER CACHE)
    UNSET(${name}_INC_DIR CACHE)

    FOREACH (p_hdr ${pkg_hdr})
      FIND_PATH(${name}_INC_DIR
        NAMES ${p_hdr}
        HINTS "${${name}_INCLUDE_DIR}" "${${name}_DIR}/include"
      )
      IF (${name}_INC_DIR)
        SET (p_HEADER ${p_hdr})
        SET (${name}_HEADER ${p_hdr} PARENT_SCOPE)
        BREAK ()
      ENDIF ()
    ENDFOREACH ()

    FIND_LIBRARY(${name}_LIB
      NAMES ${pkg_libname}
      HINTS "${${name}_LIB_DIR}" "${${name}_DIR}/lib"
    )

    IF((${name}_INC_DIR) AND (${name}_LIB))
      MESSAGE(STATUS "Found ${name}: include: '${${name}_INC_DIR}/${p_HEADER}' library: '${${name}_LIB}'")
    ELSE()
      MESSAGE(FATAL_ERROR "Required ${name} include/library not found")
    ENDIF()
  ENDIF()
ENDFUNCTION()
