
SET(errorlist_SOURCES
  ErrorFilter.hh
  ErrorFilter.cc
  ErrorList.cc
  ErrorListModel.cc
)

QT4_WRAP_CPP(errorlist_MOC
  ErrorList.hh
  ErrorListModel.hh
)

ADD_LIBRARY(errorlist STATIC
  ${errorlist_SOURCES}
  ${errorlist_MOC}
)

#ADD_SUBDIRECTORY(test)
