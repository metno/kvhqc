INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)

FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

QT4_WRAP_CPP(test_common_MOC
  SqliteAccess.hh
)

ADD_LIBRARY(test_common
#  CountDataChanged.hh
#  CountDataChanged.cc
  FakeKvApp.hh
  FakeKvApp.cc
#  FakeModelAccess.hh
#  FakeModelAccess.cc
  FakeReinserter.hh
  FakeReinserter.cc
  TestHelpers.hh
  TestHelpers.cc

  SqliteAccess.hh
  SqliteAccessPrivate.hh
  SqliteAccess.cc

  load_examples_201303.cc
  load_1650_20130130.cc
  load_18210_20130410.cc
  load_18700_20141001.cc
  load_31850_20121130.cc
  load_32780_20121207.cc
  load_52640_20121231.cc
  load_54420_20121130.cc
  load_44160_20121207.cc
  load_84070_20120930.cc

  ${test_common_MOC}
)

LINK_LIBRARIES(
  test_common
  common
  util
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${GTEST_LIBRARY}
)

ADD_EXECUTABLE(common_test
#  DataListModelTest.cc
  EditAccessTest.cc
#  EditTimeColumnTest.cc
#  KvalobsAccessTest.cc
  KvHelpersTest.cc
  TimeRangeTest.cc
#  VxColumnTest.cc
#  ObsTableModelTest.cc

  CachingAccessTest.cc
  IndexBufferTest.cc
  SetDifferencesTest.cc
  SyncRequestTest.cc

  gtestMain.cc
)
ADD_TEST(NAME common_test
  COMMAND common_test --gtest_color=yes
)

QT4_WRAP_CPP(test_common_gui_MOC
  SensorChooserTest.hh
)
ADD_EXECUTABLE(common_gui_test
  SensorChooserTest.cc
  qtestMain.cc
  ${test_common_gui_MOC}
)
ADD_TEST(NAME common_gui_test
  COMMAND common_gui_test
)
