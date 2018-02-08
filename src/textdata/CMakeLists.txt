
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

QT5_CREATE_TRANSLATION(textdata_QM
  ${textdata_SOURCES}
  ${textdata_UIS}

  textdata_nb.ts
  textdata_de.ts
)
INSTALL(FILES ${textdata_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

ADD_LIBRARY(textdata
  ${textdata_SOURCES}
  ${textdata_MOC}
  ${textdata_UIH}
  ${textdata_QM}
)
