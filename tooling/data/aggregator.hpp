#pragma once

#include "types.hpp"
#include <algorithm>
#include <memory>

namespace data {

template<class T>
struct name_compare_t
{
    bool operator()(const std::unique_ptr<const T>& lhs, const std::unique_ptr<const T>& rhs) const
    {
        return lhs->get_name() < rhs->get_name();
    }
};

class reflection_aggregator_t
{
public:
    template<class T>
    decltype(auto) emplace(std::unique_ptr<const T> element)
    {
        auto [it, success] = fetch<T>().emplace(std::move(element));
        return it->get();
    }

    template<class T, class... Args>
    decltype(auto) create(Args&&... args)
    {
        auto [it, success] = fetch<T>().emplace(std::make_unique<T>(std::forward<Args>(args)...));
        return it->get();
    }

    template<class T>
    decltype(auto) fetch() const
    {
        return std::get<entry_container_t<T>>(m_entries);
    }

    template<class T>
    using entry_pointer_t = std::unique_ptr<const T>;

    template<class T>
    using entry_container_t = std::set<entry_pointer_t<T>, name_compare_t<T>>;

private:
    template<class T>
    decltype(auto) fetch()
    {
        return std::get<entry_container_t<T>>(m_entries);
    }

    template<class... T>
    using entries_t = std::tuple<entry_container_t<T>...>;

    entries_t<record_t, enum_t, builtin_t, reference_t, pointer_t, cv_qualified_type_t, constant_array_t> m_entries;
};

}