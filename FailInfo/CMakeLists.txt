
SET(fail_HEADERS
  src/FailList.h
  src/FailDialog.h)

SET(fail_SOURCES
  src/cFailedParam.cc
  src/FailList.cc
  src/explainQC.cc
  src/FailDialog.cc)

# src/main.cc
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/authentication
)

QT4_WRAP_CPP(fail_HEADERS_MOC ${fail_HEADERS})

ADD_LIBRARY(failinfo STATIC
  ${fail_SOURCES} 
  ${fail_HEADERS_MOC})
