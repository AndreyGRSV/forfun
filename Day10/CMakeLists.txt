# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lglpk")
add_executable(puzzle10 "puzzle10.cpp")
add_executable(puzzle10_2_glpk "puzzle10_2_glpk.cpp")

target_link_libraries(puzzle10_2_glpk PRIVATE glpk)

