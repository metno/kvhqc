
SET(fail_HEADERS
  src/FailList.h
  src/FailDialog.h)

SET(fail_SOURCES
  src/cFailedParam.cc
  src/FailList.cc
  src/explainQC.cc
  src/FailDialog.cc)

SET(fail_UIS
    src/cfailedwidget.ui
)
QT4_WRAP_UI(fail_UIS_H ${fail_UIS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/authentication
)

QT4_WRAP_CPP(fail_HEADERS_MOC ${fail_HEADERS})

message(${fail_UIS_H})
ADD_LIBRARY(failinfo STATIC
  ${fail_SOURCES} 
  ${fail_HEADERS_MOC}
  ${fail_UIS_H})
