
SET(rejectedobs_SOURCES
  RejectedObs.cc
  RejectedObsDialog.cc
  RejectedQueryTask.cc
)

QT5_WRAP_CPP(rejectedobs_MOC
  RejectedObs.hh
  RejectedObsDialog.hh
)

QT5_CREATE_TRANSLATION(rejectedobs_QM
  ${rejectedobs_SOURCES}
  ${rejectedobs_UIS}

  rejectedobs_nb.ts
  rejectedobs_de.ts
)
INSTALL(FILES ${rejectedobs_QM} DESTINATION "${HQC_INSTALL_SHAREDIR}/lang")

ADD_LIBRARY(rejectedobs
  ${rejectedobs_SOURCES}
  ${rejectedobs_MOC}
  ${rejectedobs_UIH}
  ${rejectedobs_QM}
)
