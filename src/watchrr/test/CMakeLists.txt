INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

LINK_LIBRARIES(
  test_common
  watchrr
  common
  util
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${GTEST_LIBRARY}
)

ADD_EXECUTABLE(watchrr_test
  AnalyseFCCTest.cc
  AnalyseRR24Test.cc
  TaskAccessTest.cc
  gtestMain.cc
)

#ADD_TEST(NAME watchrr_test
#  COMMAND watchrr_test --gtest_color=yes
#)
