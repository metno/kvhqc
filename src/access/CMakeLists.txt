
SET(access_SOURCES
  BaseRequest.hh
  BaseRequest.cc
  IndexBuffer.hh
  IndexBuffer.cc
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
  CachingAccess.cc
  CachingAccess.hh
  CachingAccessPrivate.hh
  SyncRequest.cc
  SyncRequest.hh
  SyncRequestPrivate.hh
)

QT4_WRAP_CPP(access_MOC
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
