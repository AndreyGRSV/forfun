cmake_minimum_required(VERSION "3.50")

#Preffered compiler clang++
#set(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(puzzles2025 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(COMMON_HEADERS common/common.h)

add_executable(puzzle1 "Day 1/puzzle1.cpp" ${COMMON_HEADERS})
add_executable(puzzle2 "Day 2/puzzle2.cpp" ${COMMON_HEADERS})
add_executable(puzzle3 "Day 3/puzzle3.cpp" ${COMMON_HEADERS})
add_executable(puzzle4 "Day 4/puzzle4.cpp" ${COMMON_HEADERS})
add_executable(puzzle5 "Day 5/puzzle5.cpp" ${COMMON_HEADERS})
add_executable(puzzle6 "Day 6/puzzle6.cpp" ${COMMON_HEADERS})
add_executable(puzzle6_2 "Day 6/puzzle6_2.cpp" ${COMMON_HEADERS})
add_executable(puzzle7 "Day 7/puzzle7.cpp" ${COMMON_HEADERS})


target_compile_definitions(puzzle1 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 1/input")
target_compile_definitions(puzzle2 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 2/input")
target_compile_definitions(puzzle3 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 3/input")
target_compile_definitions(puzzle4 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 4/input")
target_compile_definitions(puzzle5 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 5/input")
target_compile_definitions(puzzle6 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 6/input")
target_compile_definitions(puzzle6_2 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 6/input")
target_compile_definitions(puzzle7 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day 7/input")
