INCLUDE_DIRECTORIES(#"${CMAKE_SOURCE_DIR}/Watch"
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
#  # include directory for generated files
#  "${CMAKE_CURRENT_BINARY_DIR}/algorithms"
)

FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)

FIND_PACKAGE(GTest QUIET)
IF(NOT GTEST_FOUND)
  MESSAGE("apparently no compiled GTest library, trying to build it")
  FIND_FILE(GTEST_DIR src/gtest-all.cc /usr/src/gtest /usr/local/src/gtest)
  IF(NOT GTEST_DIR)
    MESSAGE(FATAL_ERROR "could not find gtest-all.cc")
  ENDIF()
  GET_FILENAME_COMPONENT(GTEST_DIR ${GTEST_DIR} PATH)
  GET_FILENAME_COMPONENT(GTEST_DIR ${GTEST_DIR} PATH)
  ADD_SUBDIRECTORY(${GTEST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
  SET(GTEST_LIBRARY gtest)
  SET(GTEST_MAIN_LIBRARY gtest_main)
ENDIF()

FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)

ADD_LIBRARY(watchrr2test
  FakeDataAccess.hh
  FakeDataAccess.cc
  FakeKvApp.hh
  FakeKvApp.cc
  FakeModelAccess.hh
  FakeModelAccess.cc
  TestHelpers.hh
  TestHelpers.cc
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
  ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

ADD_EXECUTABLE(watchRR2maintest
  testMain.cc
)

ADD_EXECUTABLE(watchRR2test
  AnalyseFCCTest.cc
  AnalyseRR24Test.cc
  EditTimeColumnTest.cc
#  FakeDataAccessTest.cc
  HelpersTest.cc
  VxColumnTest.cc
)

ADD_TEST(NAME watchRR2test
  COMMAND watchRR2test --gtest_color=yes)
