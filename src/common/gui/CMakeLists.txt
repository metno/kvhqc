
SET(common_gui_SOURCES
)

QT4_WRAP_CPP(common_gui_MOC
)

ADD_LIBRARY(common_gui STATIC
  ${common_gui_SOURCES}
  ${common_gui_MOC}
)
