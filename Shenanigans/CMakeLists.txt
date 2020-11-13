cmake_minimum_required(VERSION 3.17)
project(Shenanigans)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-fmem-report")
set(CMAKE_CXX_FLAGS "-std=c++17")

# Make sure Metabench can be found when writing include(metabench)
list(APPEND CMAKE_MODULE_PATH "../metabench")

# Actually include the module
include(../metabench/metabench.cmake)

# Add new datasets
metabench_add_dataset(sortUnique "dataset1.cpp.erb" "(1..40).to_a")
metabench_add_dataset(handRolled "dataset2.cpp.erb" "(1..40).to_a")

# Add a new chart
metabench_add_chart(chart DATASETS sortUnique handRolled)

add_executable(Shenanigans test.cpp)
add_executable(Shenanigans2 debugSample.cpp)
#target_compile_options(Shenanigans2 PRIVATE "-fmem-report")
