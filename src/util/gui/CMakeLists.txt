
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
)

SET(util_gui_SOURCES
  BusyIndicator.cc
  EtaProgressBar.cc
  EtaProgressDialog.cc
  FrozenColumnTableView.cc
  FrozenRowTableView.cc
  HideApplyBox.cc
  HintWidget.cc
  ItemCheckBox.cc
  MiDateTimeEdit.cc
  QSmallTable.hh
  ToolTipStringListModel.cc
  UiHelpers.cc
)

QT4_WRAP_CPP(util_gui_MOC
  EtaProgressBar.hh
  FrozenColumnTableView.hh
  FrozenRowTableView.hh
  HideApplyBox.hh
  HintWidget.hh
  MiDateTimeEdit.hh
)

QT4_WRAP_UI(util_gui_UIH
  hideapplybox.ui
)

ADD_LIBRARY(util_gui STATIC
  ${util_gui_SOURCES}
  ${util_gui_MOC}
  ${util_gui_UIH}
)
