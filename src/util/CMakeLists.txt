
SET(util_SOURCES
  ChangeReplay.hh
  Helpers.cc
  HqcLogging.cc
  hqc_paths.cc
  Milog4cpp.cc
  stringutil.cc
  timeutil.cc
  ValueChange.hh
  VersionedValue.hh

  ActionButton.cc
  AnimatedLabel.cc
  BusyLabel.cc
  CurrentTableView.cc
  EtaProgressBar.cc
  EtaProgressDialog.cc
  FrozenColumnTableView.cc
  FrozenRowTableView.cc
  HideApplyBox.cc
  ItemCheckBox.cc
  MiDateTimeEdit.cc
  QSmallTable.hh
  Synchronizer.cc
  ToolTipStringListModel.cc
  UiHelpers.cc
  VisibleWidget.cc
)

QT4_WRAP_CPP(util_MOC
  ActionButton.hh
  AnimatedLabel.hh
  BusyLabel.hh
  CurrentTableView.hh
  EtaProgressBar.hh
  FrozenColumnTableView.hh
  FrozenRowTableView.hh
  HideApplyBox.hh
  MiDateTimeEdit.hh
  Synchronizer.hh
  VisibleWidget.hh
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
