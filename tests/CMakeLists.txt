
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
    piece_table_insert_empty.cpp
    piece_table_insert_start.cpp
    piece_table_insert_middle.cpp
    piece_table_insert_end.cpp
)

add_executable(reped_tests
  ${TEST_SOURCES}
)

target_link_libraries(reped_tests
  GTest::gtest_main
  GTest::gmock
  reped_lib
)

include(GoogleTest)
gtest_discover_tests(reped_tests)