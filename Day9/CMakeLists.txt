#apt install libomp-22-dev
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp ")
add_executable(puzzle9 "puzzle9.cpp")
add_executable(puzzle9_2 "puzzle9_2.cpp")

target_compile_definitions(puzzle9 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day9/input")
target_compile_definitions(puzzle9_2 PRIVATE INPUT_FILE="${CMAKE_SOURCE_DIR}/Day9/input")

# Find OpenMP
find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp ")
    target_link_libraries(puzzle9_2 PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found - puzzle9_2 will use parallel processing")
else()
    message(WARNING "OpenMP not found - puzzle9_2 will run sequentially")
endif()

