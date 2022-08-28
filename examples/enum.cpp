#include <tsmp/reflect.hpp>

int main(int, char*[]) {

    enum class eclass : std::uint32_t {
        value1 = 1,
        value2 = 42,
        value3 = 1337
    };

    constexpr auto values = tsmp::enum_values<eclass>;
    static_assert(values[0] == eclass::value1);
    static_assert(values[1] == eclass::value2);
    static_assert(values[2] == eclass::value3);
    constexpr auto names = tsmp::enum_names<eclass>;
    static_assert(names[0] == "value1");
    static_assert(names[1] == "value2");
    static_assert(names[2] == "value3");
    static_assert(tsmp::enum_to_string(eclass::value1) == "value1");
    static_assert(tsmp::enum_to_string(eclass::value2) == "value2");
    static_assert(tsmp::enum_to_string(eclass::value3) == "value3");
    static_assert(tsmp::enum_from_string<eclass>("value1") == eclass::value1);
    static_assert(tsmp::enum_from_string<eclass>("value2") == eclass::value2);
    static_assert(tsmp::enum_from_string<eclass>("value3") == eclass::value3);

    return 0;
}