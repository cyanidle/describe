#include <describe/describe.hpp>
#include <string>

struct BIG {};
struct small {};
struct validator {};

template<int min, int max>
struct in_range : validator {
    bool check(int value) {
        return min < value && value < max;
    }
};

struct Data {
    std::string name;
    int value;
};

DESCRIBE_ATTRS(Data, BIG, in_range<0, 7>)
DESCRIBE(Data, &_::name, &_::value)

using missing = describe::extract_attr_t<small, Data>;
static_assert(std::is_same_v<missing, void>);

using found = describe::extract_attr_t<BIG, Data>;
static_assert(std::is_same_v<found, BIG>);

// attr is considered found if it is a subclass!
using object_check = describe::extract_attr_t<validator, Data>;
static_assert(std::is_same_v<object_check, in_range<0, 7>>);

using all = describe::get_attrs_t<Data>;
static_assert(std::is_same_v<all, describe::Attrs<BIG, in_range<0, 7>>>);

// Fields inherit attributes from class
constexpr auto a = describe::Get<Data>().get<0>();
using all_a = describe::get_attrs_t<decltype(a)>;
static_assert(std::is_same_v<all, all_a>);

// Unless specified for field individually
struct custom {};

constexpr auto b = describe::Get<Data>().get<0>();
using all_b = describe::get_attrs_t<decltype(a)>;
static_assert(std::is_same_v<all, all_a>);

