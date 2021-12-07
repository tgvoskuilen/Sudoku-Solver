// SudokuSolver.cpp : Defines the entry point for the application.
//

#include "SudokuSolver.h"
#include <sstream>
#include <iostream>
#include <array>
#include <stdlib.h>
#include <string>
#include <bitset>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <assert.h>

using Entry = unsigned short;
using Entries = std::array<Entry, 81>;

constexpr Entry base_mask = (1 << 9) - 1; // 111111111

namespace {
    inline unsigned count_bits(Entry i) {
        assert(i <= base_mask);
        unsigned count = 0;
        while (i) {
            i &= (i - 1);
            ++count;
        }
        return count;
    }

    inline bool has_bit(Entry e, int i) {
        assert(i < 10 && i > 0);
        assert(e <= base_mask);
        const Entry mask = (1 << (i - 1));
        return (mask & e) > 0;
    }

    inline unsigned lowest_bit(Entry i) {
        assert(i <= base_mask);

        if (i == 0) return 0;

        unsigned bit = 1;
        while ((i & 1) == 0) {
            i = (i >> 1);
            ++bit;
        }
        return bit;
    }

    inline bool has_single_value(Entry i) {
        return count_bits(i) == 1;
    }

    inline bool remove_values(Entry& e, Entry i) {
        assert(e <= base_mask);
        assert(i <= base_mask);
        const Entry eold = e;
        e &= ((~i) & base_mask);
        assert(e <= base_mask);
        return e != eold;
    }

    inline std::string entity_bits(Entry i) {
        std::ostringstream bits;
        std::bitset<32> y(i);
        bits << y;
        return bits.str();
    }

    std::string entity_str(Entry i) {
        std::ostringstream p;
        if (has_single_value(i)) {
            p << lowest_bit(i);
        }
        else {
            p << "_";
        }

        return p.str();
    }
}

class Puzzle {
public:
    Puzzle(const std::string& init) : init_(init) {
        if (init.size() != 81) {
            throw std::runtime_error("Invalid puzzle size");
        }

        for (int i = 0; i < 81; ++i) {
            char c = init[i];
            unsigned long j = std::atoi(&c);
            if (j > 0) {
                entries[i] = (1 << (j - 1));
            }
            else {
                entries[i] = (1 << 9) - 1;
            }
        }

        init_arrays();
    }

    Puzzle(const Puzzle& p) = delete;
    Puzzle& operator=(const Puzzle& p) = delete;

    std::string to_string() {
        std::ostringstream p;

        p << "+-------+-------+-------+\n";

        for (int i = 0; i < 9; ++i) {
            p << "|";
            for (int j = 0; j < 9; ++j) {
                const int idx = 9 * i + j;
                p << " " << entity_str(entries[idx]);
                if ((j + 1) % 3 == 0) p << " |";
            }
            if ((i + 1) % 3 == 0) {
                p << "\n+-------+-------+-------+\n";
            }
            else {
                p << "\n";
            }
        }

        return p.str();
    }

    void solve() {
        int tries = 0;
        auto start = std::chrono::steady_clock::now();

        try {

            while (!puzzle_complete()) {
                ++tries;

                if (tries > 1000000) {
                    throw std::runtime_error("Could not solve puzzle within iteration limit");
                }

                if (rule1()) {
                    if (!is_valid()) revert_guess();
                    continue;
                }

                if (rule2()) {
                    if (!is_valid()) revert_guess();
                    continue;
                }

                if (rule3()) {
                    if (!is_valid()) revert_guess();
                	continue;
                }

                guess();
            }

            auto end = std::chrono::steady_clock::now();
            elapsed = 1e-3*std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            solved_ = true;
            std::cout << "Solved " << init_ << " in " << elapsed << " ms" << std::endl;
        }
        catch (std::exception& e) {
            std::ostringstream msg;
            msg << "FATAL ERROR IN SOLVE after step " << tries << " guess " << num_guesses() << std::endl;
            msg << init_ << std::endl;
            msg << e.what() << std::endl;
            for (auto&& e : entries) {
                msg << " " << e << ": " << entity_bits(e) << std::endl;
            }
            error_ = msg.str();
        }
    }

    bool solved() const { return solved_; }
    double elapsed_time() const { return elapsed; }
    std::string to_code() const { return init_; }
    int num_guesses() const { return num_guesses_;  }
    std::string error() const { return error_; }

private:

    std::string error_ = "";

    bool rule1() {
        /*
        If an entry has a given value, that value
        can be eliminated from all other entries in the 
        sets that entry belongs to
        */
        bool changed = false;
        for (int i = 0; i < 81; ++i) {
            if (has_single_value(entries[i])) {
                for (auto j : entity_sets[i]) {
                    for (auto ei : sets[j]) {
                        if (ei != i) {
                            changed |= remove_values(entries[ei], entries[i]);
                        }
                    }
                }
            }
        }

        return changed;

    }

    bool rule2() {
        /*
        If a set has only one location where a given number can go, it
        must go there
        */
        bool changed = false;

        for (int i = 1; i < 10; ++i) {
            for (auto&& set : sets) {
                int num_matches = 0;
                int match_id = -1;

                for (auto ei : set) {
                    if (has_bit(entries[ei], i)) {
                        ++num_matches;
                        match_id = ei;
                    }
                }

                if (num_matches == 1 && !has_single_value(entries[match_id])) {
                    entries[match_id] = (1 << (i - 1));
                    changed = true;
                }

            }
        }

        return changed;
    }

    bool rule3() {
        /*
        If a set has a group of N entries which each have the same
        N number options (and no others) then those N numbers can
        be removed from all the other entries in the set
        */
        bool changed = false;

        for (auto&& set : sets) {
            for (auto ei : set) {

                // which other entries in this set have the same values as entry i
                unsigned num_matches = 0;
                for (auto ej : set) {
                    if (entries[ej] == entries[ei]) {
                        ++num_matches;
                    }
                }

                if (num_matches > 1 && count_bits(entries[ei]) == num_matches) {
                    for (auto ej : set) {
                        if (entries[ej] != entries[ei]) {
                            changed |= remove_values(entries[ej], entries[ei]);
                        }
                    }
                }
            }
        }


        return changed;
    }

    bool rule4() {
        /*
        If N values within a set are only present in N entries, then
        all other options from those entries can be eliminated
        */
        bool changed = false;

        // Too complicated to code


        return changed;
    }

    bool rule5() {
        /*
        If all possible spots for integer i in set J are also
        in set K, then all other instances of i from set K can
        be eliminated
        */
        bool changed = false;

        std::array<unsigned, 27> match_ids{};

        // works, but is more expensive than just extra guessing

        for (int j = 0; j < 27; ++j) {
            for (int i = 1; i < 10; ++i) {

                int n = 0;

                // entry ids in sets[j] that contain value i
                for (auto ei : sets[j]) {
                    if (has_bit(entries[ei], i) && !has_single_value(entries[ei])) {
                        match_ids[n++] = ei;
                    }
                }

                if (n > 0) {
                    for (int k = 0; k < 27; ++k) {
                        if (k == j) continue;
                        
                        bool all_found = true;
                        for (int m = 0; m < n; ++m) {
                            if (std::find(sets[k].begin(), sets[k].end(), match_ids[m]) == sets[k].end()) {
                                all_found = false;
                                break;
                            }
                        }

                        if (!all_found) continue;
                        
                        // set k contains all the match ids, remove i from all the non-matched ids in set k
                        auto* begin = match_ids.data();
                        auto* end = begin + n;
                        const Entry iVal = (1 << (i - 1));

                        for (auto ei : sets[k]) {
                            if (std::find(begin, end, ei) == end) {
                                changed |= remove_values(entries[ei], iVal);
                            }
                        }

                    }
                }
            }
        }


        return changed;
    }

    void guess() {
        /*
        Pick a cell with the minimum number of choices, save the current state,
        and make a guess. Eliminate the guessed value from the saved state so
        if we have to revert, we don't guess the same thing
        */
        ++num_guesses_;

        unsigned min_bits = 100;
        int guess_id = -1;

        // TODO: pick with minimum guesses that has maximum elimination of other candidates

        for (int i = 0; i < 81; ++i) {
            if (!has_single_value(entries[i])) {
                const unsigned num_options = count_bits(entries[i]);
                if (num_options < min_bits) {
                    min_bits = num_options;
                    guess_id = i;
                }
            }
        }

        if (guess_id < 0) {
            throw std::runtime_error("Reached invalid state in guessing routine - nothing left to guess");
        }

        const Entry guessed_entity = entries[guess_id];
        const unsigned guess_value = lowest_bit(guessed_entity);

        if (guess_value > 9) {
            std::ostringstream msg;
            msg << "Got an invalid guess of " << guess_value << " from " << entity_bits(guessed_entity);
            throw std::runtime_error(msg.str());
        }
        const Entry guess_mask = (1 << (guess_value - 1));

        guesses.push_back(entries);
        remove_values(guesses.back()[guess_id], guess_mask);
        entries[guess_id] = guess_mask;
    }

    void revert_guess() {
        /*
        If a solution cannot be found, revert to the state before the
        most recent guess
        */
        if (guesses.empty()) {
            throw std::runtime_error("Solution failed, no more guesses to revert");
        }

        entries = guesses.back();
        guesses.pop_back();
    }

    bool set_complete(const std::array<unsigned,9>& set) {

        Entry mask = 0;

        for (auto ei : set) {
            if (!has_single_value(entries[ei])) {
                return false;
            }
            else {
                mask ^= entries[ei];
            }
        }

        if (mask == base_mask) {
            return true;
        }
        else {
            bool valid = is_valid();

            Entry expected = 0;
            Entry actual = 0;
            int num_vals = 0;
            for (auto ei : set) {
                if (has_single_value(entries[ei])) {
                    expected |= entries[ei];
                    actual ^= entries[ei];
                    ++num_vals;
                }
            }

            bool mismatch = num_vals != count_bits(expected);
            std::stringstream msg;
            msg << "Puzzle has entered an invalid state in set, mask = " << entity_bits(mask) << ", valid = " << valid << std::endl;
            msg << "  Actual = " << entity_bits(actual) << std::endl;
            msg << "  Expect = " << entity_bits(expected) << ", mismatch = " << mismatch << std::endl;
            for (auto ei : set) {
                bool is_zero = (entries[ei] == 0);
                msg << ei << ": " << entity_bits(entries[ei]) << " is_zero:" << is_zero << " raw:" << entries[ei] << std::endl;
            }

            throw std::runtime_error(msg.str());
        }

    }

    bool puzzle_complete() {
        for (auto&& set : sets) {
            if (!set_complete(set)) {
                return false;
            }
        }
        return true;
    }

    bool is_valid() {

        // not valid if any Entry has 0 remaining options
        constexpr Entry zero = 0;
        for (auto e : entries) {
            if (e > base_mask) {
                std::ostringstream msg;
                msg << "Value exceeded base " << e << " " << entity_bits(e) << std::endl;
                throw std::runtime_error(msg.str());
            }

            if (e == zero) {
                return false;
            }
        }

        // not valid if a set has duplicate defined options
        for (auto&& set : sets) {
            Entry expected = 0;
            Entry actual = 0;
            unsigned num_vals = 0;
            for (auto ei : set) {
                if (has_single_value(entries[ei])) {
                    expected |= entries[ei];
                    actual ^= entries[ei];
                    ++num_vals;
                }
            }

            const bool count_mismatch = num_vals != count_bits(expected);

            if (expected != actual || count_mismatch ) {
                return false;
            }
        }

        return true;
    }


    void init_arrays() {

        int k = 0;

        for (int i = 0; i < 9; ++i) {
            std::array<unsigned, 9> s{};
            for (int j = 0; j < 9; ++j) s[j] = 9 * i + j;
            sets.push_back(s);
        }

        for (int i = 0; i < 9; ++i) {
            std::array<unsigned, 9> s{};
            for (int j = 0; j < 9; ++j) s[j] = 9 * j + i;
            sets.push_back(s);
        }

        for (int i = 0; i < 9; i += 3) {
            for (int j = 0; j < 9; j += 3) {
                unsigned ib = 9 * i + j;
                sets.push_back({ ib, ib + 1, ib + 2, ib + 9, ib + 10, ib + 11, ib + 18, ib + 19, ib + 20 });
            }
        }

        for (int i = 0; i < 81; ++i) {
            for (int j = 0; j < 27; ++j) {
                if (std::find(sets[j].begin(), sets[j].end(), i) != sets[j].end()) {
                    entity_sets[i].push_back(j);
                }
            }
        }

    }

    std::vector<std::array<unsigned, 9>> sets{};
    std::array<std::vector<unsigned>, 81> entity_sets{};

    std::string init_;
    Entries entries;
    std::vector<Entries> guesses;
    unsigned num_guesses_ = 0;
    double elapsed = 0.0;
    bool solved_ = false;
};

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
        puzzles[i]->solve();
        if (!puzzles[i]->solved()) {
            max_runs = i+1;
            break;
        }
    }

    double avg_time = 0.0;
    double avg_guesses = 0.0;
    int num_errs = 0;
    for (int i = 0; i < max_runs; ++i) {
        if (puzzles[i]->solved()) {
            avg_time += puzzles[i]->elapsed_time();
            avg_guesses += puzzles[i]->num_guesses();
        }
        else {
            ++num_errs;
        }
    }
    avg_time /= max_runs;
    avg_guesses /= max_runs;

    std::cout << "Solved " << max_runs << " puzzles, average time = " << avg_time << " ms, avg guesses = " << avg_guesses << std::endl;

    if (num_errs > 0) {
        std::cout << "FAILED to solve " << num_errs << " puzzles:" << std::endl;
        for (int i = 0; i < max_runs; ++i) {
            if (!puzzles[i]->solved()) {
                std::cout << puzzles[i]->to_code() << std::endl;
                std::cout << puzzles[i]->error() << std::endl;
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
        //pv[i]->to_string();
        pv[i]->solve();


        if (!pv[i]->solved()) {
            std::cout << pv[i]->to_string() << std::endl;
            std::cout << pv[i]->error() << std::endl;
            //std::cout << pv[i]->init_vals << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    int max_runs = 10000000;
    if (argc == 2) {
        max_runs = std::atoi(argv[1]);
    }

    if (!test_archive(max_runs)) {
        spot_test({
            "........8..3...4...9..2..6.....79.......612...6.5.2.7...8...5...1.....2.4.5.....3",
            "........2..8.1.9..5....3.4....1.93...6..3..8...37......4......53.1.7.8..2........",
            "..2...7...1.....6.5......18....37.......49.....41.23....3.2.9...8.....5.6.......2",
            "........7..4.2.6..8.....31......29...4..9..3...95.6....1......8..6.5.2..7......6."
        });
    }

    return 0;
}
