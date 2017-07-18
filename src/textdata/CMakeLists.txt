
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

ADD_LIBRARY(textdata
  ${textdata_SOURCES}
  ${textdata_MOC}
  ${textdata_UIH}
  ${textdata_QM}
)
