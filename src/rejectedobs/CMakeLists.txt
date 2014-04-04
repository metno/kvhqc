
SET(rejectedobs_SOURCES
  RejectedObs.cc
  RejectedObsDialog.cc
)

QT4_WRAP_CPP(rejectedobs_MOC
  RejectedObs.hh
  RejectedObsDialog.hh
)

ADD_LIBRARY(rejectedobs
  ${rejectedobs_SOURCES}
  ${rejectedobs_MOC}
  ${rejectedobs_UIH}
  ${rejectedobs_QM}
)
