
SET(util_SOURCES
  ChangeReplay.hh
  lru_cache.hh

  Helpers.cc
  HqcLogging.cc
  hqc_paths.cc
  hqcObsPgm.cc
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

QT5_WRAP_CPP(util_MOC
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

SET(util_UIS
  hideapplybox.ui
)
QT5_WRAP_UI(util_UIH ${util_UIS})

METNO_QT5_CREATE_TRANSLATION(util_QM
  UPDATE_SOURCES
  ${util_SOURCES}
  ${util_UIS}

  TSFILES
  util_nb.ts
  util_de.ts

  UPDATE_TARGET
  lupdate
)

INSTALL(FILES ${util_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

ADD_LIBRARY(util STATIC
  ${util_SOURCES}
  ${util_MOC}
  ${util_UIH}
  ${util_QM}
)

ADD_SUBDIRECTORY(test)
