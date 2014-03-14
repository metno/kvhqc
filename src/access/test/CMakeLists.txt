INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)

FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

QT4_WRAP_CPP(test_access_MOC
  SqliteAccess.hh
)

ADD_LIBRARY(test_access
  SqliteAccess.hh
  SqliteAccessPrivate.hh
  SqliteAccess.cc

  ${test_access_MOC}
)

LINK_LIBRARIES(
  test_access
  access
  common
  util_gui
  util
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${GTEST_LIBRARY}
)

ADD_EXECUTABLE(access_test
  CachingAccessTest.cc
  IndexBufferTest.cc
  SetDifferencesTest.cc
  SyncRequestTest.cc

  gtestMain.cc
)

ADD_TEST(NAME access_test
  COMMAND access_test --gtest_color=yes
)
