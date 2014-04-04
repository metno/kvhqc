
SET(errors_SOURCES
  ErrorFilter.hh
  ErrorFilter.cc
  ErrorList.cc
  ErrorListModel.cc
)

QT4_WRAP_CPP(errors_MOC
  ErrorList.hh
  ErrorListModel.hh
)

ADD_LIBRARY(errors STATIC
  ${errors_SOURCES}
  ${errors_MOC}
)

#ADD_SUBDIRECTORY(test)
