#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.hpp"
#include "describe/describe.hpp"

struct MetaPair {
    std::string_view meta, name;
};

static MetaPair next(std::string_view& src) {
    MetaPair res;
    describe::detail::get_next_stripped(src, &res.meta, &res.name);
    return res;
}
namespace a {

#define optional _

struct Lol {
    int a, b;
};
DESCRIBE(a::Lol, &_::a, &optional::b)

}


TEST_CASE("describe") {
    SUBCASE("get_next_stripped") {
        std::string_view body = "&_::f,       &optional::m";
        auto f = next(body);
        auto m = next(body);
        CHECK(f.meta == "_");
        CHECK(f.name == "f");
        CHECK(m.meta == "optional");
        CHECK(m.name == "m");
    }
    SUBCASE("macro") {
        auto desc = describe::Get<a::Lol>();
        auto a = desc.get<0>();
        auto b = desc.get<1>();
        CHECK(desc.cls_name == "Lol");
        CHECK(desc.ns_name == "a");
        CHECK(a.name == "a");
        CHECK(a.meta == "_");
        CHECK(b.name == "b");
        CHECK(b.meta == "optional");
    }
}


