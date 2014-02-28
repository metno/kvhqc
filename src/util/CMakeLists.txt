
SET(util_SOURCES
  ChangeReplay.hh
  Helpers.cc
  HqcLogging.cc
  hqc_paths.cc
  Milog4cpp.cc
  timeutil.cc
  ValueChange.hh
  VersionedValue.hh
)

ADD_LIBRARY(util STATIC
  ${util_SOURCES}
)

ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(test)
