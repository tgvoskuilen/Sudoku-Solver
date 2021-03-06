#pragma once

#include <array>
#include <string>
#include <vector>

using Entry = unsigned short;
using Entries = std::array<Entry,81>;

constexpr Entry base_mask = 0b0111111111;
constexpr Entry lock_mask = 0b1000000000;

class Puzzle {
public:
    Puzzle(const std::string& init, bool quiet = false);
    Puzzle(const Puzzle& p) = delete;
    Puzzle& operator=(const Puzzle& p) = delete;

    void summarize() const;
    void solve();
    void solve_recurse();

    bool solved() const { return solved_; }
    double elapsed_time() const { return elapsed; }
    std::string initial_state() const { return init_; }
    int num_guesses() const { return num_guesses_; }

    friend std::ostream& operator<<(std::ostream& os, const Puzzle& p);

private:
    void print(std::ostream& os) const;

    bool recurse(Entries values);
    bool rule1();
    bool rule2();
    bool rule3();
    bool rule4();
    bool rule5();
    void guess();
    void revert_guess();
    bool set_complete(const std::array<unsigned, 9>& set) const;
    bool puzzle_complete() const;
    bool is_valid() const;

    std::array<Entry, 9> columns{};
    std::array<unsigned, 9> match_ids{};
    std::array<unsigned, 9> match_count{};
    std::array<unsigned, 6> calls_{};
    std::array<unsigned, 6> applies_{};

    std::string init_;
    Entries entries;
    std::vector<Entries> guesses;

    unsigned num_guesses_ = 0;
    double elapsed = 0.0;
    bool solved_ = false;
    const bool quiet_ = false;

    static constexpr std::array<std::array<unsigned, 3>, 81> entity_sets = { {
        { 0, 9,18}, { 0,10,18}, { 0,11,18},
        { 0,12,19}, { 0,13,19}, { 0,14,19},
        { 0,15,20}, { 0,16,20}, { 0,17,20},

        { 1, 9,18}, { 1,10,18}, { 1,11,18},
        { 1,12,19}, { 1,13,19}, { 1,14,19},
        { 1,15,20}, { 1,16,20}, { 1,17,20},

        { 2, 9,18}, { 2,10,18}, { 2,11,18},
        { 2,12,19}, { 2,13,19}, { 2,14,19},
        { 2,15,20}, { 2,16,20}, { 2,17,20},

        { 3, 9,21}, { 3,10,21}, { 3,11,21},
        { 3,12,22}, { 3,13,22}, { 3,14,22},
        { 3,15,23}, { 3,16,23}, { 3,17,23},

        { 4, 9,21}, { 4,10,21}, { 4,11,21},
        { 4,12,22}, { 4,13,22}, { 4,14,22},
        { 4,15,23}, { 4,16,23}, { 4,17,23},

        { 5, 9,21}, { 5,10,21}, { 5,11,21},
        { 5,12,22}, { 5,13,22}, { 5,14,22},
        { 5,15,23}, { 5,16,23}, { 5,17,23},

        { 6, 9,24}, { 6,10,24}, { 6,11,24},
        { 6,12,25}, { 6,13,25}, { 6,14,25},
        { 6,15,26}, { 6,16,26}, { 6,17,26},

        { 7, 9,24}, { 7,10,24}, { 7,11,24},
        { 7,12,25}, { 7,13,25}, { 7,14,25},
        { 7,15,26}, { 7,16,26}, { 7,17,26},

        { 8, 9,24}, { 8,10,24}, { 8,11,24},
        { 8,12,25}, { 8,13,25}, { 8,14,25},
        { 8,15,26}, { 8,16,26}, { 8,17,26}
    } };

    static constexpr std::array<std::array<unsigned, 9>, 27> sets = { {
        { 0, 1, 2,  3, 4, 5,  6, 7, 8}, // 0
        { 9,10,11, 12,13,14, 15,16,17},
        {18,19,20, 21,22,23, 24,25,26},
        {27,28,29, 30,31,32, 33,34,35},
        {36,37,38, 39,40,41, 42,43,44},
        {45,46,47, 48,49,50, 51,52,53},
        {54,55,56, 57,58,59, 60,61,62},
        {63,64,65, 66,67,68, 69,70,71},
        {72,73,74, 75,76,77, 78,79,80},

        { 0, 9,18, 27,36,45, 54,63,72}, // 9
        { 1,10,19, 28,37,46, 55,64,73},
        { 2,11,20, 29,38,47, 56,65,74},
        { 3,12,21, 30,39,48, 57,66,75},
        { 4,13,22, 31,40,49, 58,67,76},
        { 5,14,23, 32,41,50, 59,68,77},
        { 6,15,24, 33,42,51, 60,69,78},
        { 7,16,25, 34,43,52, 61,70,79},
        { 8,17,26, 35,44,53, 62,71,80},

        { 0, 1, 2,  9,10,11, 18,19,20}, // 18
        { 3, 4, 5, 12,13,14, 21,22,23},
        { 6, 7, 8, 15,16,17, 24,25,26},
        {27,28,29, 36,37,38, 45,46,47},
        {30,31,32, 39,40,41, 48,49,50},
        {33,34,35, 42,43,44, 51,52,53},
        {54,55,56, 63,64,65, 72,73,74},
        {57,58,59, 66,67,68, 75,76,77},
        {60,61,62, 69,70,71, 78,79,80}
    }};
};

std::ostream& operator<<(std::ostream& os, const Puzzle& p);
