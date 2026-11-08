set(ORBBEC_SDK_ROOT "/home/junjun/tools/pyorbbecsdk/sdk" CACHE PATH
  "Orbbec SDK root containing include/libobsensor and lib/linux_x64")
set(ORBBEC_HEADER "${ORBBEC_SDK_ROOT}/include/libobsensor/ObSensor.hpp")
set(ORBBEC_LIBRARY "${ORBBEC_SDK_ROOT}/lib/linux_x64/libOrbbecSDK.so")

if(NOT EXISTS "${ORBBEC_HEADER}" OR NOT EXISTS "${ORBBEC_LIBRARY}")
  message(STATUS "Orbbec SDK not found; the wrist DC1 target is not built")
  return()
endif()

add_library(dual_nero_dabai_dc1 src/dabai_dc1.cpp)
target_include_directories(dual_nero_dabai_dc1
  PUBLIC include
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
  PRIVATE ${ORBBEC_SDK_ROOT}/include)
target_link_libraries(dual_nero_dabai_dc1 PUBLIC ${ORBBEC_LIBRARY})
target_link_options(dual_nero_dabai_dc1 PUBLIC "LINKER:-rpath,${ORBBEC_SDK_ROOT}/lib/linux_x64")
target_compile_options(dual_nero_dabai_dc1 PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
  add_executable(dabai_dc1_test tests/dabai_dc1_test.cpp)
  target_compile_options(dabai_dc1_test PRIVATE -Wall -Wextra -Wpedantic)
  target_link_libraries(dabai_dc1_test PRIVATE dual_nero_dabai_dc1 GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(dabai_dc1_test)
endif()
