#pragma once

#include <set>

namespace std {
// This is a implementation of std::erase_if from C++20. If the introspect tool can be compiled with C++20, 
// this function can be deleted. Look here: https://en.cppreference.com/w/cpp/container/set/erase_if
template< class Key, class Compare, class Alloc, class Pred >
typename std::set<Key,Compare,Alloc>::size_type erase_if( std::set<Key,Compare,Alloc>& c, Pred pred ) {
    auto old_size = c.size();
    for (auto i = c.begin(), last = c.end(); i != last; ) {
    if (pred(*i)) {
        i = c.erase(i);
    } else {
        ++i;
    }
    }
    return old_size - c.size();
}

}