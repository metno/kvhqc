
SET(textdata_SOURCES
  GetTextData.cc
  GetTextData.h
  TextdataDialog.cc
  TextdataTable.cc
  TxtDat.hh
)

QT4_WRAP_CPP(textdata_MOC
  TextdataDialog.hh
  TextdataTable.hh
)

ADD_LIBRARY(textdata
  ${textdata_SOURCES}
  ${textdata_MOC}
  ${textdata_UIH}
  ${textdata_QM}
)
