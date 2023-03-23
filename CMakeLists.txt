cmake_minimum_required(VERSION 3.0)

project(tinyraytracer-C LANGUAGES C)

add_executable(tinyraytracer-C tinyraytracer.c)

target_compile_options(tinyraytracer-C PRIVATE -Wall -Wextra -O3)

target_compile_features(tinyraytracer-C PRIVATE c_std_99)

target_link_libraries(tinyraytracer-C PRIVATE m)

find_package(OpenMP REQUIRED)
target_link_libraries(tinyraytracer-C PRIVATE OpenMP::OpenMP_C)
