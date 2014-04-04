INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

LINK_LIBRARIES(
  test_common
  watchrr
)

ADD_EXECUTABLE(watchrr_test
  AnalyseFCCTest.cc
  AnalyseRR24Test.cc
)

ADD_TEST(NAME watchrr_test
  COMMAND watchrr_test --gtest_color=yes
)
