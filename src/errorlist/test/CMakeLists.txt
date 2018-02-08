INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

LINK_LIBRARIES(
  test_common
  errorlist
  common
  util
  ${PC_LIBRARIES}
  ${OTHER_LIBS}
  ${GTEST_LIBRARY}
)

ADD_EXECUTABLE(errorlist_test
  ErrorListTest.cc
  load_3200_20141008.cc
  gtestMain.cc
)
ADD_TEST(NAME errorlist_test
  COMMAND errorlist_test --gtest_color=yes
)
