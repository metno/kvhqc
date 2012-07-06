
SET(auth_HEADERS
  Authenticator.h
  authenticationdialog.h)

SET(auth_SOURCES
  hqc_paths.cc
  Authenticator.cc
  authenticationdialog.cc)

QT4_WRAP_CPP(auth_HEADERS_MOC ${auth_HEADERS})

ADD_LIBRARY(authentication STATIC
  ${auth_SOURCES} 
  ${auth_HEADERS_MOC})
