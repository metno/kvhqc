
SET(textdata_SOURCES
  TextdataDialog.cc
  TextdataTable.cc
  TextDataQueryTask.cc
  TextDataQueryTask.hh
  TxtDat.hh
)

QT5_WRAP_CPP(textdata_MOC
  TextdataDialog.hh
  TextdataTable.hh
)

METNO_QT5_CREATE_TRANSLATION(textdata_QM
  UPDATE_SOURCES
  ${textdata_SOURCES}
  ${textdata_UIS}

  TSFILES
  textdata_nb.ts
  textdata_de.ts

  UPDATE_TARGET
  lupdate
)

INSTALL(FILES ${textdata_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

ADD_LIBRARY(textdata
  ${textdata_SOURCES}
  ${textdata_MOC}
  ${textdata_UIH}
  ${textdata_QM}
)
