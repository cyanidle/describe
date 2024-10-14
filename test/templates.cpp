#include <describe/describe.hpp>

namespace test {

template<typename T>
struct Data {
    T a;
    T b[10];
};

// all macros work just fine. you just must define template<> above them
template<typename T>
DESCRIBE(test::Data<T>, &_::a, &_::b)

}

constexpr auto desc = describe::Get<test::Data<int>>();
static_assert(desc.name == "test::Data<T>");
constexpr auto B = describe::by_index<1>(desc);
static_assert(std::is_same_v<decltype(B)::type, int[10]>);


template<auto a, auto b> struct sum {
    static constexpr auto value = a + b;
    int as_int = int(value);
};

// DESCRIBE_CLASS(...) DESCRIBE_FIELDS(...) pair for multi-param templates
template<auto a, auto b>
DESCRIBE_TEMPL_CLASS(sum<a, b>)
DESCRIBE_TEMPL_FIELDS(&_::as_int)


constexpr auto sum_desc = describe::Get<sum<1, 2>>();
static_assert(sum_desc.name == "sum<a, b>");
