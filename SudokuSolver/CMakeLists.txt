# CMakeList.txt : CMake project for SudokuSolver, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (SudokuSolver "SudokuSolver.cpp"  "Puzzle.h" "Puzzle.cpp" "tests/test_solve.cpp")
set_property(TARGET SudokuSolver PROPERTY CXX_STANDARD 17)

# TODO: Add tests and install targets if needed.
# enable_testing()
