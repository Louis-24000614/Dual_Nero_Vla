find_package(PkgConfig REQUIRED)
pkg_check_modules(REALSENSE2 IMPORTED_TARGET realsense2)

if(NOT REALSENSE2_FOUND)
  message(STATUS "librealsense2 not found; the environment D435 target is not built")
  return()
endif()

add_library(dual_nero_realsense_d435 src/realsense_d435.cpp)
target_include_directories(dual_nero_realsense_d435
  PUBLIC include
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(dual_nero_realsense_d435 PUBLIC PkgConfig::REALSENSE2)
target_compile_options(dual_nero_realsense_d435 PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
  add_executable(realsense_d435_test tests/realsense_d435_test.cpp)
  target_compile_options(realsense_d435_test PRIVATE -Wall -Wextra -Wpedantic)
  target_link_libraries(realsense_d435_test PRIVATE dual_nero_realsense_d435 GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(realsense_d435_test)
endif()
