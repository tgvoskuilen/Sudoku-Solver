// SudokuSolver.cpp : Defines the entry point for the application.
//

#include "Puzzle.h"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <assert.h>
#include <algorithm>


std::vector<std::unique_ptr<Puzzle>> read_puzzles() {

    std::vector<std::string> files = { "puzzles6_forum_hardest_1106", "puzzles2_17_clue","puzzles3_magictour_top1465" };

    std::vector<std::unique_ptr<Puzzle>> p;

    for (auto&& f : files) {
        std::ifstream pfile(f);
        if (pfile.is_open()) {
            std::string line;
            while (std::getline(pfile, line)) {
                if (line.rfind("#", 0) == 0) continue;

                if (line.size() == 81) {
                    p.emplace_back(std::make_unique<Puzzle>(line));
                }
            }
            pfile.close();
        }
        else {
            std::cout << "COULD NOT OPEN FILE " << f << std::endl;
        }
    }

    std::cout << "Read " << p.size() << " puzzles" << std::endl;

    return p;
}

bool test_archive(int max_runs) {
    auto puzzles = read_puzzles();

    if (puzzles.empty()) return false;

    max_runs = std::min(max_runs, (int)puzzles.size());

    for (int i = 0; i < max_runs; ++i) {
        puzzles[i]->solve_recurse();
        if (!puzzles[i]->solved()) {
            max_runs = i+1;
            break;
        }
    }



    double avg_time = 0.0;
    double avg_guesses = 0.0;
    int num_errs = 0;
    int no_guess_solves = 0;
    int max_guesses = 0;
    double min_time = 1e12;
    double max_time = 0.0;

    for (int i = 0; i < max_runs; ++i) {
        if (puzzles[i]->solved()) {
            avg_time += puzzles[i]->elapsed_time();
            avg_guesses += puzzles[i]->num_guesses();

            if (puzzles[i]->num_guesses() == 0) ++no_guess_solves;
            max_guesses = std::max(max_guesses, puzzles[i]->num_guesses());

            min_time = std::min(min_time, puzzles[i]->elapsed_time());
            max_time = std::max(max_time, puzzles[i]->elapsed_time());
        }
        else {
            ++num_errs;
        }
    }
    avg_time /= max_runs;
    avg_guesses /= max_runs;



    std::cout << "Solved " << max_runs << " puzzles, average time = " << avg_time << " ms, avg guesses = " << avg_guesses << std::endl;

    std::cout << "  No-guess solves: " << no_guess_solves << " max guesses: " << max_guesses << std::endl;
    std::cout << "  Min time " << min_time << " ms, max time " << max_time << " ms" << std::endl;

    // sort by num guesses
    std::sort(puzzles.begin(), puzzles.end(), [](const auto& lhs, const auto& rhs) {
        return lhs->num_guesses() > rhs->num_guesses();
        });

    std::cout << "10 hardest puzzles by guess count" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << puzzles[i]->initial_state() << ": " << puzzles[i]->num_guesses() << " guesses" << std::endl;
    }

    // sort by time
    std::sort(puzzles.begin(), puzzles.end(), [](const auto& lhs, const auto& rhs) {
        return lhs->elapsed_time() > rhs->elapsed_time();
        });

    std::cout << "10 hardest puzzles by solve time" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << puzzles[i]->initial_state() << ": " << puzzles[i]->elapsed_time() << " ms" << std::endl;
    }


    if (num_errs > 0) {
        std::cout << "FAILED to solve " << num_errs << " puzzles:" << std::endl;
        for (int i = 0; i < max_runs; ++i) {
            if (!puzzles[i]->solved()) {
                std::cout << puzzles[i]->initial_state() << std::endl;
            }
        }
    }

    return true;
}

void spot_test(const std::vector<std::string>& pl) {
    std::vector<std::unique_ptr<Puzzle>> pv;
    for (auto&& p : pl) {
        pv.push_back(std::make_unique<Puzzle>(p));
    }
    
    for (int i = 0; i < (int)pv.size(); ++i) {
        //std::cout << pv[i]->to_string() << std::endl;
        pv[i]->solve_recurse();
    }

    //std::cout << "SUMMARY" << std::endl;
    //for (int i = 0; i < (int)pv.size(); ++i) {
    //    pv[i]->summarize();
    //}
}



int main(int argc, char* argv[])
{
    int max_runs = 10000000;
    if (argc == 2) {
        max_runs = std::atoi(argv[1]);
    }

    if (!test_archive(max_runs)) {
        spot_test({
            "1.......2..34...5..6....7.....85..9....3.6.....8.9.....2....1..7.......6..9.8..3.",
            "........8..3...4...9..2..6.....79.......612...6.5.2.7...8...5...1.....2.4.5.....3",
            "........2..8.1.9..5....3.4....1.93...6..3..8...37......4......53.1.7.8..2........",
            "..2...7...1.....6.5......18....37.......49.....41.23....3.2.9...8.....5.6.......2",
            "........7..4.2.6..8.....31......29...4..9..3...95.6....1......8..6.5.2..7......6."
        });
    }

    return 0;
}
