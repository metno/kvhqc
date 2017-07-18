
SET(rejectedobs_SOURCES
  RejectedObs.cc
  RejectedObsDialog.cc
  RejectedQueryTask.cc
)

QT5_WRAP_CPP(rejectedobs_MOC
  RejectedObs.hh
  RejectedObsDialog.hh
)

ADD_LIBRARY(rejectedobs
  ${rejectedobs_SOURCES}
  ${rejectedobs_MOC}
  ${rejectedobs_UIH}
  ${rejectedobs_QM}
)
