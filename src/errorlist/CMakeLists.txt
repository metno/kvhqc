
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

QT4_WRAP_UI(errorlist_UIH
  errorlist.ui
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(errorlist STATIC
  ${errorlist_SOURCES}
  ${errorlist_MOC}
  ${errorlist_UIH}
)

#ADD_SUBDIRECTORY(test)
