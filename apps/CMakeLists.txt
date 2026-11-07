add_executable(record_can record_can.cpp)
target_compile_options(record_can PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(record_can PRIVATE dual_nero_can_transport)
