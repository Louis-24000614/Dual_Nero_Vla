cmake_minimum_required(VERSION 3.20)

project(dual_nero_vla LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
find_package(GTest REQUIRED)

# Modules are added in dependency order.
add_subdirectory(modules/arm)
add_subdirectory(modules/cameras)
add_subdirectory(apps)
