#include "test_macros.h"
#include <iostream>
#include "../Puzzle.h"
#include <vector>

TEST(Puzzle_SolveHardest) {
    std::vector<std::string> puzzles = { {
        "..39.....4...8..36..8...1...4..6..738......1......2.....4.7..686........7.....5..",
        "1....6.8....7..1....9.....4.......5..18..5...5..36.8..6.5..8.3.8....3.1.....2....",
        "1....6.8....7..1....9.....4.......5..18..5...5..36....6.5..8.3.8....3.1.....2...8",
        "....9..5..1.....3...23..7....45...7.8.....2.......64...9..1.....8..6......54....7",
        "................12..3..4..5.....6.......7.3..128..........2......9...4...6.15....",
        "..3......4...8..36..8...1...4..6..73...9..........2.....4.7..686...2....7..6..5..",
        "........9.5.7...2.7.9..2....1.67..5.......4..8....5....7.31....6....7.3..3..6...1",
        "......7....71.9...68..7......1.6785.5....3.....8.1.9....6.9.1...4.....9.........2",
        ".2.4...8...7.....3.8.237.1.2.1....9..9....8.4...9......1.8...4.5.8..........6....",
        ".2.4...8...7.....3.8.237.1.2.1....9..9....8.4...9......1.8...4.5............6...8"
    } };

    for (auto& ps : puzzles) {
        Puzzle p(ps, true);
        p.solve();
        EXPECT_TRUE(p.solved());
    }
}

TEST(Puzzle_SolveHardestRecurse) {
    std::vector<std::string> puzzles = { {
        "..39.....4...8..36..8...1...4..6..738......1......2.....4.7..686........7.....5..",
        "1....6.8....7..1....9.....4.......5..18..5...5..36.8..6.5..8.3.8....3.1.....2....",
        "1....6.8....7..1....9.....4.......5..18..5...5..36....6.5..8.3.8....3.1.....2...8",
        "....9..5..1.....3...23..7....45...7.8.....2.......64...9..1.....8..6......54....7",
        "................12..3..4..5.....6.......7.3..128..........2......9...4...6.15....",
        "..3......4...8..36..8...1...4..6..73...9..........2.....4.7..686...2....7..6..5..",
        "........9.5.7...2.7.9..2....1.67..5.......4..8....5....7.31....6....7.3..3..6...1",
        "......7....71.9...68..7......1.6785.5....3.....8.1.9....6.9.1...4.....9.........2",
        ".2.4...8...7.....3.8.237.1.2.1....9..9....8.4...9......1.8...4.5.8..........6....",
        ".2.4...8...7.....3.8.237.1.2.1....9..9....8.4...9......1.8...4.5............6...8"
    } };

    for (auto& ps : puzzles) {
        Puzzle p(ps, true);
        p.solve_recurse();
        EXPECT_TRUE(p.solved());
    }
}