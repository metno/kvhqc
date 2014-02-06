INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)

FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(test_common
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
  test_common
  common
  common_gui
  util
  util_gui
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${GTEST_LIBRARY}
)

ADD_EXECUTABLE(common_test
  AnalyseFCCTest.cc
  AnalyseRR24Test.cc
  DataListModelTest.cc
  EditAccessTest.cc
  EditTimeColumnTest.cc
  KvalobsAccessTest.cc
  KvHelpersTest.cc
  TimeRangeTest.cc
  VxColumnTest.cc
  ObsTableModelTest.cc
  gtestMain.cc
)

ADD_TEST(NAME common_test
  COMMAND common_test --gtest_color=yes
)
