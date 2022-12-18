#include "data/types.hpp"
#include <catch2/catch_all.hpp>

#include <data/prefix_splitter.hpp>

using namespace data;

TEST_CASE("prefix_splitter_t simple construction", "[core][unit]") {
    const std::vector<record_decl_t> records {
        record_decl_t{ "foo_t", { field_decl_t{"i"} }, {} },
        record_decl_t{ "bar_t", { field_decl_t{"j"} }, {} }
    };
    data::prefix_splitter_t splitter{ records };

    REQUIRE(splitter.fields()[0].name == "i");
    REQUIRE(splitter.fields()[1].name == "j");

    REQUIRE(splitter.records()[0].name == "foo_t");
    REQUIRE(splitter.records()[1].name == "bar_t");
}

TEST_CASE("prefix_splitter_t simple construction 2", "[core][unit]") {
    const std::vector<record_decl_t> records {
        record_decl_t{ "foo_t", {}, { function_decl_t{"i"} } },
        record_decl_t{ "bar_t", {}, { function_decl_t{"j"} } }
    };
    data::prefix_splitter_t splitter{ records };

    REQUIRE(splitter.functions()[0].name == "i");
    REQUIRE(splitter.functions()[1].name == "j");

    REQUIRE(splitter.records()[0].name == "foo_t");
    REQUIRE(splitter.records()[1].name == "bar_t");
}

TEST_CASE("prefix_splitter_t zero fields construction", "[core][unit]") {
    const std::vector<record_decl_t> records {
        record_decl_t{ "foo_t", {}, {} },
        record_decl_t{ "bar_t", {}, {} }
    };
    data::prefix_splitter_t splitter{ records };

    REQUIRE(splitter.fields().size() == 0);

    REQUIRE(splitter.records().size() == 1);
    REQUIRE(splitter.records()[0].name == "<unknown>");
}

TEST_CASE("prefix_splitter_t shared  field construction", "[core][unit]") {
    const std::vector<record_decl_t> records {
        record_decl_t{ "foo_t", { field_decl_t{"i"}, field_decl_t{"j"} }, {} },
        record_decl_t{ "bar_t", { field_decl_t{"i"}, field_decl_t{"k"} }, {} }
    };
    data::prefix_splitter_t splitter{ records };

    REQUIRE(splitter.fields()[0].name == "i");
    REQUIRE(splitter.fields()[1].name == "j");
    REQUIRE(splitter.fields()[2].name == "k");

    REQUIRE(splitter.records()[0].name == "foo_t");
    REQUIRE(splitter.records()[1].name == "bar_t");
}

TEST_CASE("prefix_splitter_t matching fields construction", "[core][unit]") {
    const std::vector<record_decl_t> records {
        record_decl_t{ "foo_t", { field_decl_t{"i"}, field_decl_t{"j"} }, {} },
        record_decl_t{ "bar_t", { field_decl_t{"i"}, field_decl_t{"j"} }, {} }
    };
    data::prefix_splitter_t splitter{ records };

    REQUIRE(splitter.fields()[0].name == "i");
    REQUIRE(splitter.fields()[1].name == "j");

    REQUIRE(splitter.records().size() == 1);
    REQUIRE(splitter.records()[0].name == "<unknown>");
}