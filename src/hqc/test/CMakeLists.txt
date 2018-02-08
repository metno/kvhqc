
INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

ADD_DEFINITIONS(-DTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  -DTOP_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

QT5_WRAP_CPP(test_hqc_gui_MOC
  SimpleCorrectionsTest.hh
)
ADD_EXECUTABLE(hqc_gui_test
  SimpleCorrectionsTest.cc
  qtestMain.cc
  ${test_hqc_gui_MOC}
)
TARGET_LINK_LIBRARIES(hqc_gui_test
  test_common
  hqc_common
  common
  util
  ${PC_LIBRARIES}
  ${OTHER_LIBS}
  ${QT_LIBRARIES}
)

## do not include this test in "make check" to prevent problems when
## building packages without access to an X server
# ADD_TEST(NAME hqc_gui_test
#   COMMAND hqc_gui_test
# )
