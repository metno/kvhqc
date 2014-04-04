
SET(util_SOURCES
  ChangeReplay.hh
  Helpers.cc
  HqcLogging.cc
  hqc_paths.cc
  Milog4cpp.cc
  timeutil.cc
  ValueChange.hh
  VersionedValue.hh

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

QT4_WRAP_CPP(util_MOC
  EtaProgressBar.hh
  FrozenColumnTableView.hh
  FrozenRowTableView.hh
  HideApplyBox.hh
  HintWidget.hh
  MiDateTimeEdit.hh
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
QT4_WRAP_UI(util_UIH
  hideapplybox.ui
)

ADD_LIBRARY(util STATIC
  ${util_SOURCES}
  ${util_MOC}
  ${util_UIH}
)

ADD_SUBDIRECTORY(test)
