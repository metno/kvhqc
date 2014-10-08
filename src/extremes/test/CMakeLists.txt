INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

LINK_LIBRARIES(
  test_common
  extremes
  common
  util
  ${PC_KVCPP_LIBRARIES}
  ${PC_METLIBS_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${GTEST_LIBRARY}
)

ADD_EXECUTABLE(extremes_test
  ExtremesTest.cc
  load_17000_20141002.cc
  gtestMain.cc
)
ADD_TEST(NAME extremes_test
  COMMAND extremes_test --gtest_color=yes
)
