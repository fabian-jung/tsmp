#include <fmt/format.h>
#include <functional>
#include <map>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <chrono>

#include "tsmp/introspect.hpp"
#include "tsmp/proxy.hpp"
#include "tsmp/reflect.hpp"

struct entry_t
{
    std::chrono::duration<double> duration;
    size_t calls;
};

struct trace_data_t
{
    struct collector_t
    {
        ~collector_t()
        {
            const auto end = std::chrono::high_resolution_clock::now();
            data.duration += end - begin;
            ++data.calls;
        }
        entry_t& data;
        const std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    };

    trace_data_t(std::string class_name)
        : class_name(std::move(class_name))
    {
    }

    collector_t collect(std::string name) { return {data[std::move(name)]}; }

    ~trace_data_t()
    {
        for (const auto& [name, entry] : data) {
            fmt::print("{}::{} took {}s in {} calls\n", class_name, name, entry.duration.count(), entry.calls);
        }
    }

    std::string class_name;
    std::map<std::string, entry_t> data;
};

struct tracing_functor_t
{

    tracing_functor_t(const char* class_name)
        : data(class_name)
    {
    }

    trace_data_t data;

    template<class... Args>
    decltype(auto) operator()(auto& fn, std::string name, Args&&... args)
    {
        const auto collector = data.collect(std::move(name));
        return std::invoke(fn, std::forward<Args>(args)...);
    }
};

template<class T>
using tracing_proxy = tsmp::value_proxy<T, tracing_functor_t>;

int main(int, char*[])
{
    auto test_vector = tsmp::value_proxy{std::vector<int>{}, tracing_functor_t{"std::vector<int>"}};

    for (auto i = 0u; i < 1000000u; ++i) {
        test_vector.push_back(5);
    }

    for (auto i = 0u; i < 1000000u; ++i) {
        test_vector.front() = 5;
    }

    for (auto i = 0u; i < 1000000u; ++i) {
        test_vector.at(0) = 5;
    }

    return 0;
}