INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
  "${CMAKE_SOURCE_DIR}/include"
)

FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)

FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(watchrr2test
  CountDataChanged.hh
  CountDataChanged.cc
  FakeKvApp.hh
  FakeKvApp.cc
  FakeModelAccess.hh
  FakeModelAccess.cc
  FakeReinserter.hh
  FakeReinserter.cc
  TestHelpers.hh
  TestHelpers.cc
  load_examples_201303.cc
  load_1650_20130130.cc
  load_18210_20130410.cc
  load_31850_20121130.cc
  load_32780_20121207.cc
  load_52640_20121231.cc
  load_54420_20121130.cc
  load_44160_20121207.cc
  load_84070_20120930.cc)

LINK_LIBRARIES(
  watchrr2test
  watchrr2
  authentication
  ${PC_KVCPP_LIBRARIES} ${PC_METLIBSQT_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${GTEST_LIBRARY})

ADD_EXECUTABLE(watchRR2maintest
  testMain.cc
)

ADD_EXECUTABLE(watchRR2test
  AnalyseFCCTest.cc
  AnalyseRR24Test.cc
  ChangeReplayTest.cc
  EditAccessTest.cc
  EditTimeColumnTest.cc
  KvalobsAccessTest.cc
  HelpersTest.cc
  VersionedValueTest.cc
  VxColumnTest.cc
  gtestMain.cc)

ADD_TEST(NAME watchRR2test
  COMMAND watchRR2test --gtest_color=yes)
