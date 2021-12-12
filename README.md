# Sudoku Solver

At this point, who hasn't made a Sudoku solver? The first one I made was a cobbled together Excel+Visual Basic version many years ago.

This solver has both a backtracking option and a rule-based solver (that falls back to guesses when the rules fail). The rule based solver is, on average, about 300x faster than the backtracking method.

The puzzle state is stored as an array of 81 unsigned shorts, with the lower 9 bits indicating which numbers can go in that spot.