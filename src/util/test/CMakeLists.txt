INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
  "${GTEST_DIR}/include"
)

ADD_EXECUTABLE(util_test
  ChangeReplayTest.cc
  HelpersTest.cc
  VersionedValueTest.cc
)

TARGET_LINK_LIBRARIES(util_test
  util
  kvalobs_miconfparser boost_system
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${GTEST_LIBRARY}
  ${GTEST_MAIN_LIBRARY}
)

ADD_TEST(NAME util_test
  COMMAND util_test --gtest_color=yes
)
