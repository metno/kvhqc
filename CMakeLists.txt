# HQC - Free Software for Manual Quality Control of Meteorological Observations
#
# Copyright (C) 2012-2018 met.no
#
# Contact information:
# Norwegian Meteorological Institute
# Box 43 Blindern
# 0313 OSLO
# NORWAY
# email: kvalobs-dev@met.no
#
# This file is part of HQC
#
# HQC is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# HQC is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with HQC; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

PROJECT(kvhqc)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(MetnoCMakeSetup)
INCLUDE(MetnoFindPackage)
INCLUDE(MetnoGTest)
INCLUDE(MetnoQtTranslation)
INCLUDE(MetnoVersions)

METNO_CMAKE_SETUP()
METNO_PVERSION(4.1.0~a31)

ENABLE_TESTING()

SET(HQC_INSTALL_SHAREDIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}/${PVERSION}")

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(share)

INSTALL(FILES etc/README DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/kvhqc/${PVERSION}")

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
  CONFIGURE_FILE("Doxyfile.in" "Doxyfile" @ONLY)
  ADD_CUSTOM_TARGET(doc
    ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation with Doxygen" VERBATIM
    )
ENDIF()
