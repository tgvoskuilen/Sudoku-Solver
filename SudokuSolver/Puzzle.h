#pragma once

#include <array>
#include <string>
#include <vector>

using Entry = unsigned short;
using Entries = std::array<Entry, 81>;

class Puzzle {
public:
    Puzzle(const std::string& init);
    Puzzle(const Puzzle& p) = delete;
    Puzzle& operator=(const Puzzle& p) = delete;

    std::string to_string() const;
    void summarize() const;
    void solve();

    bool solved() const { return solved_; }
    double elapsed_time() const { return elapsed; }
    std::string to_code() const { return init_; }
    int num_guesses() const { return num_guesses_; }

private:
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

    void init_arrays();

    std::array<Entry, 9> columns{};
    std::array<unsigned, 9> match_ids{};
    std::array<unsigned, 9> match_count{};
    std::array<unsigned, 6> calls_{};
    std::array<unsigned, 6> applies_{};

    std::vector<std::array<unsigned, 9>> sets{};
    std::array<std::vector<unsigned>, 81> entity_sets{};

    std::string init_;
    Entries entries;
    std::vector<Entries> guesses;

    unsigned num_guesses_ = 0;
    double elapsed = 0.0;
    bool solved_ = false;
};