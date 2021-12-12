# CMakeList.txt : CMake project for SudokuSolver, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (SudokuSolver "SudokuSolver.cpp"  "Puzzle.h" "Puzzle.cpp" "bit_ops.h")
set_property(TARGET SudokuSolver PROPERTY CXX_STANDARD 17)

# TODO: Add tests and install targets if needed.
enable_testing()
#add_subdirectory("tests")

add_executable( unitTests "tests/test_main.cpp" "Puzzle.h" "Puzzle.cpp" "tests/test_macros.h" "tests/test_bit_ops.cpp" "bit_ops.h" "tests/test_solve.cpp" "tests/test_rules.cpp")
add_test( basic_test unitTests )

#find_package(GTest REQUIRED)