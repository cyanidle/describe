#include <describe/describe.hpp>
#include <string>

// Attributes are just structs!
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

DESCRIBE("Data", Data, BIG, in_range<0, 7>, in_range<0, 6>, in_range<0, 5>) {
    MEMBER("name", &_::name);
    MEMBER("value", &_::value);
}

using namespace describe;

using missing = extract_t<small, Data>;
static_assert(std::is_same_v<missing, void>);

using found = extract_t<BIG, Data>;
static_assert(std::is_same_v<found, BIG>);

// attr is considered found if it is a subclass!
using object_check = extract_t<validator, Data>;
static_assert(std::is_same_v<object_check, in_range<0, 7>>);

using all_validators = extract_all_t<validator, Data>;
static_assert(std::is_same_v<all_validators, TypeList<in_range<0, 7>, in_range<0, 6>, in_range<0, 5>>>);

using all = get_attrs_t<Data>;
static_assert(std::is_same_v<all, TypeList<BIG, in_range<0, 7>, in_range<0, 6>, in_range<0, 5>>>);


