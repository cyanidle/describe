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

#define attrs(...) &_

struct Lol {
    int a, b;
    Lol* c;
};
DESCRIBE(a::Lol, &_::a, attrs(bar|baz)::b, attrs(x > 123)::c)

}


TEST_CASE("describe") {
    SUBCASE("get_next_stripped") {
        std::string_view body = "&_::f,       &optional::m,attrs(bar|baz)::c";
        auto f = next(body);
        auto m = next(body);
        auto c = next(body);
        CHECK(f.meta == "_");
        CHECK(f.name == "f");
        CHECK(m.meta == "optional");
        CHECK(m.name == "m");
        CHECK(c.meta == "attrs(bar|baz)");
        CHECK(c.name == "c");
    }
    SUBCASE("macro") {
        auto desc = describe::Get<a::Lol>();
        auto a = desc.get<0>();
        auto b = desc.get<1>();
        auto c = desc.get<2>();
        CHECK(desc.cls_name == "Lol");
        CHECK(desc.ns_name == "a");
        CHECK(a.name == "a");
        CHECK(a.meta == "_");
        CHECK(b.name == "b");
        CHECK(b.meta == "attrs(bar|baz)");
        CHECK(c.name == "c");
        CHECK(c.meta == "attrs(x > 123)");
    }
}

namespace a {

// defining a meta-info tag for fields
#define optional _
// you can put anything before field it just must expand to &_ in the end
#define one_of(...) &_
#define in_range(...) &_

struct My {
    int a, b;
    std::string c;
    float d;
};

// inside this macro '_' is actually an alias to a::My
DESCRIBE(a::My, &_::a, &optional::b, one_of(foo|bar|baz)::c, in_range(0 < x < 15)::d)
//       ^ namespace        ^ everything is captured Literally (without macro expansion)
//                     ^ '&' and '::' are discarded

// NOTE: ::<field_name> is required!
// Fields are separated based on commas! do not put commas inside meta data!

}

TEST_CASE("example") {
    a::My obj;
    constexpr auto desc = describe::Get<a::My>();
    // ^ this description can be used in compile-time functions!
    constexpr auto a = desc.get<0>();

    static_assert(std::is_same_v<decltype(a), const describe::Field<&a::My::a>>);

    constexpr auto b = desc.get<1>(); //these are in the same order as in DESCRIBE()
    constexpr auto c = desc.get<2>();
    constexpr auto d = desc.get<3>();
    static_assert(desc.fields_count == 4);
    static_assert(desc.cls_name == "My");
    static_assert(desc.ns_name == "a");

    static_assert(a.name == "a");
    obj.*a.value = 33; // write value
    static_assert(a.meta == "_");

    static_assert(b.name == "b");
    static_assert(b.meta == "optional");

    static_assert(c.name == "c");
    static_assert(c.meta == "one_of(foo|bar|baz)");
    using c_type = decltype(of(c)); // ADL-powered helper to get type of field;
    static_assert(std::is_same_v<c_type, std::string>);
    c_type current_value = obj.*c.value;

    static_assert(d.name == "d");
    static_assert(d.meta == "in_range(0 < x < 15)");
}
