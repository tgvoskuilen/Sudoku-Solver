#pragma once

#include "Puzzle.h"
#include <bitset>
#include <ostream>
#include <sstream>

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

inline std::string entity_str(Entry i) {
    std::ostringstream p;
    if (has_single_value(i)) {
        p << lowest_bit(i);
    }
    else {
        p << "_";
    }

    return p.str();
}

inline unsigned entry_box_id(unsigned i) {
    const unsigned box_col = (i % 9) / 3;
    const unsigned box_row = i / 27;
    return 3 * box_row + box_col;
}

