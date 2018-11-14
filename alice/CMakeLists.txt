cmake_minimum_required (VERSION 3.8)

project (alice VERSION 1.0.0)

add_compile_options(-std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -pedantic)

add_executable(alice alice.cpp)