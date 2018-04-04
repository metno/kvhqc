
SET(rejectedobs_SOURCES
  RejectedObs.cc
  RejectedObsDialog.cc
  RejectedQueryTask.cc
)

QT5_WRAP_CPP(rejectedobs_MOC
  RejectedObs.hh
  RejectedObsDialog.hh
)

METNO_QT5_CREATE_TRANSLATION(rejectedobs_QM
  UPDATE_SOURCES
  ${rejectedobs_SOURCES}
  ${rejectedobs_UIS}

  TSFILES
  rejectedobs_nb.ts
  rejectedobs_de.ts

  UPDATE_TARGET
  lupdate
)

INSTALL(FILES ${rejectedobs_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

ADD_LIBRARY(rejectedobs
  ${rejectedobs_SOURCES}
  ${rejectedobs_MOC}
  ${rejectedobs_UIH}
  ${rejectedobs_QM}
)
