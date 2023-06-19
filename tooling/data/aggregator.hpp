#pragma once

#include "types.hpp"
#include <algorithm>
#include <memory>

namespace data {

template <class T>
struct name_compare_t {
    bool operator()(const T* lhs, const T* rhs) const {
        return lhs->get_name() < rhs->get_name();
    }
};

class reflection_aggregator_t {
public:

    reflection_aggregator_t() {
        free(m_entries);
    }

    template <class T, class... Args>
    decltype(auto) create(Args&&... args) {
        auto [it, success] = fetch<T>().emplace(new T(args...));
        return *it;
    }
    
    template <class T>
    decltype(auto) fetch() const {
        return std::get<entry_container_t<T>>(m_entries);
    }

    template <class T>
    using entry_container_t = std::set<T*, name_compare_t<T>>;

private:
    template <class T>
    decltype(auto) fetch() {
        return std::get<entry_container_t<T>>(m_entries);
    }

    template <class... T>
    using entries_t = std::tuple<entry_container_t<T>...>;

    template <class T>
    void free(entry_container_t<T>& container) {
        std::for_each(container.begin(), container.end(), [](auto* ptr) { delete ptr; });
    }

    template <class... T>
    void free(std::tuple<entry_container_t<T>...>) {
        (free(fetch<T>()), ...);
    }

    entries_t<record_t, enum_t, builtin_t, reference_t, pointer_t, cv_qualified_type_t, template_specialisation_t> m_entries; 
};

}