INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

LINK_LIBRARIES(
  util
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${GTEST_LIBRARY}
  ${GTEST_MAIN_LIBRARY}
)

ADD_EXECUTABLE(util_test
  ChangeReplayTest.cc
  HelpersTest.cc
  priority_list_test.cc
  VersionedValueTest.cc
)

ADD_TEST(NAME util_test
  COMMAND util_test --gtest_color=yes
)
