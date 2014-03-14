
SET(access_SOURCES
  BackgroundAccess.hh
  BackgroundAccessPrivate.hh
  BackgroundAccess.cc
  BaseRequest.hh
  BaseRequest.cc
  IndexBuffer.hh
  IndexBuffer.cc
  KvalobsAccess.hh
  KvalobsAccessPrivate.hh
  KvalobsAccess.cc
  KvalobsData.hh
  KvalobsData.cc
  ObsAccess.hh
  ObsAccess.cc
  ObsData.hh
  ObsRequest.hh
  ObsRequest.cc
  ObsFilter.cc
  ObsFilter.hh
  SimpleBuffer.hh
  SimpleBuffer.cc
  SimpleRequest.hh
  SimpleRequest.cc
  SignalRequest.hh
  SignalRequest.cc
  TimeBuffer.hh
  TimeBuffer.cc
  CachingAccess.hh
  CachingAccessPrivate.hh
  CachingAccess.cc
  sqlutil.hh
  sqlutil.cc
  SyncRequest.hh
  SyncRequestPrivate.hh
  SyncRequest.cc
)

QT4_WRAP_CPP(access_MOC
  BackgroundAccess.hh
  BackgroundAccessPrivate.hh
  CachingAccessPrivate.hh
  SignalRequest.hh
  SimpleBuffer.hh
  SyncRequestPrivate.hh
)

ADD_LIBRARY(access STATIC
  ${access_SOURCES}
  ${access_MOC}
)

ADD_SUBDIRECTORY(test)
