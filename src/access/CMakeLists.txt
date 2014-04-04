
SET(access_SOURCES
)

QT4_WRAP_CPP(access_MOC
)

ADD_LIBRARY(access STATIC
  ${access_SOURCES}
  ${access_MOC}
)

ADD_SUBDIRECTORY(test)
