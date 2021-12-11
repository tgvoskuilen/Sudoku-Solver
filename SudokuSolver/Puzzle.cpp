
#include "Puzzle.h"
#include <stdexcept>
#include <bitset>
#include <sstream>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <assert.h>

constexpr Entry base_mask = 0b0111111111;
constexpr Entry lock_mask = 0b1000000000;


namespace {
    inline bool is_locked(const Entry& i) {
        return (i & lock_mask) > 0;
    }

    inline unsigned count_bits(Entry i) {
        i &= base_mask;
        unsigned count = 0;
        while (i) {
            i &= (i - 1);
            ++count;
        }
        return count;
    }

    inline bool has_bit(const Entry& e, const int& i) {
        const Entry mask = (1 << (i - 1));
        return (mask & e) > 0;
    }

    inline unsigned lowest_bit(Entry i) {
        i &= base_mask;

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
        if (is_locked(e)) return false;

        // e = L001100010
        // i = L000000110
        // r = L001100000
        // r = e & ((~i) & base)
        //  ignore lock bit for now??
        i &= base_mask;
        const Entry eold = e;
        e &= ((~i) & base_mask);
        return e != eold;
    }

    inline std::string entity_bits(Entry i) {
        std::ostringstream bits;
        std::bitset<10> y(i);
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


//===============================================================================
Puzzle::Puzzle(const std::string& init) : init_(init) {
    if (init.size() != 81) {
        throw std::runtime_error("Invalid puzzle size");
    }

    for (int i = 0; i < 81; ++i) {
        std::string c = init.substr(i, 1);
        unsigned long j = std::atoi(c.c_str());
        if (j > 0) {
            entries[i] = (1 << (j - 1));
        }
        else {
            entries[i] = base_mask;
        }
    }
}

//===============================================================================
std::string Puzzle::to_string() const {
    std::ostringstream p;

    p << "++=======+=======+=======++=======+=======+=======++=======+=======+=======++\n";

    for (int i = 0; i < 9; ++i) {


        for (int sr = 0; sr < 3; ++sr) {
            p << "||";
            for (int j = 0; j < 9; ++j) {
                const int idx = 9 * i + j;
                const Entry e = entries[idx];
                bool has_val = has_single_value(e);
                unsigned val = lowest_bit(e);


                for (int sc = 0; sc < 3; ++sc) {
                    const int opt = 3 * sr + sc + 1;

                    if (has_val) {
                        if (sc == 1 && sr == 1) {
                            p << " " << "\033[91m" << val << "\033[0m";
                        }
                        else {
                            p << "  ";
                        }
                    }
                    else {
                        if (has_bit(e, opt)) {
                            p << " " << "\033[90m" << opt << "\033[0m";
                        }
                        else {
                            p << "  ";
                        }
                    }
                }
                if ((j + 1) % 3 == 0) {
                    p << " ||";
                }
                else {
                    p << "\033[90m" << " |" << "\033[0m";
                }
            }

            p << "\n";

        }
        if ((i + 1) % 3 == 0) {
            p << "++=======+=======+=======++=======+=======+=======++=======+=======+=======++\n";
        }
        else {
            p << "++\033[90m-------+-------+-------\033[0m++\033[90m-------+-------+-------\033[0m++\033[90m-------+-------+-------\033[0m++\n";
        }
    }

    return p.str();
}

//===============================================================================
void Puzzle::summarize() const {
    std::cout << "\nSOLVED PUZZLE:" << std::endl;
    std::cout << to_string() << std::endl;

    std::cout << " Time: " << elapsed << " ms" << std::endl;
    std::cout << " Guesses: " << num_guesses_ << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << " Rule " << i + 1 << " ratio = " << applies_[i] << "/" << calls_[i] << std::endl;
    }
}

//===============================================================================
void Puzzle::solve() {
    int tries = 0;
    auto start = std::chrono::steady_clock::now();

    rule1();

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

            if (rule4()) {
                if (!is_valid()) revert_guess();
                continue;
            }

            if (rule5()) {
                if (!is_valid()) revert_guess();
                continue;
            }

            guess();
        }

        auto end = std::chrono::steady_clock::now();
        elapsed = 1e-3 * std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        solved_ = true;
        std::cout << "Solved " << init_ << " in " << elapsed << " ms with " << num_guesses_ << " guesses" << std::endl;
    }
    catch (std::exception& e) {
        std::ostringstream msg;
        msg << "FATAL ERROR IN SOLVE after step " << tries << " guess " << num_guesses() << std::endl;
        msg << init_ << std::endl;
        msg << e.what() << std::endl;
        msg << to_string() << std::endl;
        std::cout << msg.str() << std::endl;
    }
}

//===============================================================================
bool Puzzle::rule1() {
    /*
    If an entry has a given value, that value
    can be eliminated from all other entries in the
    sets that entry belongs to

    Once this is called on an entry it can be locked and skipped for
    future calls
    */
    bool changed = false;
    for (int i = 0; i < 81; ++i) {
        if (!is_locked(entries[i]) && has_single_value(entries[i])) {
            for (auto j : entity_sets[i]) {
                for (auto ei : sets[j]) {
                    if (ei != i) {
                        changed |= remove_values(entries[ei], entries[i]);
                    }
                }
            }
            entries[i] |= lock_mask;
        }
    }

    calls_[0] += 1;
    if (changed) applies_[0] += 1;

    return changed;
}
//===============================================================================
bool Puzzle::rule2() {
    /*
    If a set has only one location where a given number can go, it
    must go there
    */
    bool changed = false;

    for (auto&& set : sets) {

        for (int i = 0; i < 9; ++i) match_count[i] = 0;

        for (auto ei : set) {
            for (int i = 0; i < 9; ++i) {
                if (has_bit(entries[ei], i + 1)) {
                    match_ids[i] = ei;
                    match_count[i] += 1;
                }
            }
        }

        for (int i = 0; i < 9; ++i) {
            if (match_count[i] == 1 && !has_single_value(entries[match_ids[i]])) {
                entries[match_ids[i]] = (1 << i);
                changed = true;
            }
        }
    }

    calls_[1] += 1;
    if (changed) applies_[1] += 1;

    return changed;
}
//===============================================================================
bool Puzzle::rule3() {
    /*
    If a set has a group of N entries which each have the same
    N number options (and no others) then those N numbers can
    be removed from all the other entries in the set
    */
    bool changed = false;

    for (auto&& set : sets) {
        for (int i = 0; i < 9; ++i) {

            // which other entries in this set have the same values as entry i
            unsigned num_matches = 1; // always matches itself
            for (int j = i + 1; j < 9; ++j) {
                if (entries[set[j]] == entries[set[i]]) {
                    ++num_matches;
                }
            }

            if (num_matches > 1 && count_bits(entries[set[i]]) == num_matches) {
                for (int j = 0; j < 9; ++j) {
                    if (entries[set[j]] != entries[set[i]]) {
                        changed |= remove_values(entries[set[j]], entries[set[i]]);
                    }
                }
            }
        }
    }

    calls_[2] += 1;
    if (changed) applies_[2] += 1;

    return changed;
}

//===============================================================================
bool Puzzle::rule4() {
    /*
    If N values within a set are only present in N entries, then
    all other options from those entries can be eliminated
    */
    bool changed = false;

    // Example:
    //   9 8 7 6 5 4 3 2 1
    // a 1 1 0 0 0 0 0 1 0 <- clear bits 1-7
    // b 0 0 1 0 0 0 0 0 0
    // c 0 0 0 1 1 0 0 1 1
    // d 1 1 0 0 1 0 0 0 1 <- clear bits 1-7
    // e 0 0 0 0 1 1 0 1 1
    // f 0 0 0 0 0 0 1 0 0
    // ...
    // Transpose this into an Entry per column and search for duplicate columns.
    // If a column with N bits is duplicated N times, the other bits in the rows occupied
    // by those column bits can be removed


    for (auto&& set : sets) {

        std::fill(columns.begin(), columns.end(), 0);

        for (int i = 0; i < 9; ++i) { // loop rows
            const Entry e = entries[set[i]];

            for (int j = 0; j < 9; ++j) { // loop bits
                if (has_bit(e, j + 1)) {
                    columns[j] |= (1 << i);
                }
            }
        }

        for (int i = 0; i < 9; ++i) {
            const Entry colI = columns[i];
            Entry row_mask = (1 << i);

            unsigned num_matches = 1;
            for (int j = i + 1; j < 9; ++j) {
                const Entry colJ = columns[j];
                if (colI == colJ) {
                    ++num_matches;
                    row_mask |= (1 << j);
                }
            }

            if (num_matches > 1 && count_bits(colI) == num_matches) {
                for (int j = 0; j < 9; ++j) {
                    const Entry ej = entries[set[j]];
                    if (ej != row_mask && has_bit(colI, j + 1)) {
                        entries[set[j]] &= row_mask;
                        changed = true;
                    }
                }
            }
        }
    }

    calls_[3] += 1;
    if (changed) applies_[3] += 1;

    return changed;
}

//===============================================================================
bool Puzzle::rule5() {
    /*
    If all possible spots for integer i in (box) set J are also
    in (row/col) set K, then all other instances of i from set K can
    be eliminated.

    e.g. if a "1" can only go in the first row in the top left box
    (positions 0 1 2) then 1s can be eliminated from all other
    spots in the first row (positions 3-8)
    */
    bool changed = false;


    for (int j = 18; j < 27; ++j) { // loop only box sets
        for (int i = 1; i < 10; ++i) {

            int n = 0;

            // entry ids in sets[j] that contain value i
            for (auto ei : sets[j]) {
                if (has_bit(entries[ei], i)) {
                    match_ids[n++] = ei;
                }
            }

            if (n > 1) {
                // check if all the matches are in the same row or column
                const unsigned row = match_ids[0] / 9;
                const unsigned col = match_ids[0] % 9;

                int row_set = row;
                int col_set = 9 + col;

                for (int m = 1; m < n; ++m) {
                    if (match_ids[m] / 9 != row) row_set = -1;
                    if (match_ids[m] % 9 != col) col_set = -1;
                }

                const int k = (row_set >= 0) ? row_set : col_set;

                if (k >= 0) {
                    // set k (row or column) contains all the match ids, remove i from all the non-matched ids in set k
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

    calls_[4] += 1;
    if (changed) applies_[4] += 1;

    return changed;
}

//===============================================================================
void Puzzle::guess() {
    /*
    Pick a cell with the minimum number of choices, save the current state,
    and make a guess. Eliminate the guessed value from the saved state so
    if we have to revert, we don't guess the same thing
    */
    ++num_guesses_;

    unsigned min_bits = 100;
    int guess_id = -1;

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
    assert(guess_value < 10);
    const Entry guess_mask = (1 << (guess_value - 1));

    guesses.push_back(entries);
    remove_values(guesses.back()[guess_id], guess_mask);
    entries[guess_id] = guess_mask;
}

//===============================================================================
void Puzzle::revert_guess() {
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

//===============================================================================
bool Puzzle::set_complete(const std::array<unsigned, 9>& set) const {
    /*
    Check if a set is complete (has a single value in each entry)
    throw if it is complete but invalid.
    */

    Entry mask = 0;

    for (auto ei : set) {
        if (!has_single_value(entries[ei])) {
            return false;
        }
        else {
            mask ^= (entries[ei] & base_mask);
        }
    }

    if (mask == base_mask) {
        return true;
    }
    else {
        std::stringstream msg;
        msg << "Puzzle has entered an invalid state" << std::endl << to_string() << std::endl;
        throw std::runtime_error(msg.str());
    }

}

//===============================================================================
bool Puzzle::puzzle_complete() const {
    // Check if the puzzle is complete (all sets complete)
    for (auto&& set : sets) {
        if (!set_complete(set)) {
            return false;
        }
    }
    return true;
}

//===============================================================================
bool Puzzle::is_valid() const {
    /*
    Check if the puzzle is still in a valid state. Examples of an invalid
    state would be entries with no choices left, or duplicate entries in a set.
    */

    // not valid if any Entry has 0 remaining options
    constexpr Entry zero = 0;
    for (auto e : entries) {
        if ((e & base_mask) == zero) {
            return false;
        }
    }

    // not valid if a set has duplicate defined options
    for (auto&& set : sets) {
        Entry expected = 0;
        bool errs = false;
        for (auto ei : set) {
            if (has_single_value(entries[ei])) {
                errs |= (expected & (entries[ei] & base_mask)) > 0;
                if (errs) break;
                expected |= (entries[ei] & base_mask);
            }
        }

        //const bool count_mismatch = num_vals != count_bits(expected);

        if (errs) {
            //std::cout << "FOUND ERRS" << std::endl << to_string() << std::endl;
            return false;
        }
    }

    return true;
}

//===============================================================================