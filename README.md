# describe
Minimal C++17 Reflection Library in 300 LOC. 

# Features
* Constexpr field pointer + name iteration
* Inheritance
* Enums support
* Templates support
* Friend support
* Constexpr non-invasive attributes for classes (per-field customizable)

# Usage

## Basic
```cpp
#include <describe/describe.hpp>
#include <iostream>

struct Data {
    int a;
    int b;
    void method() {

    }
};

// Inside macro a '_' alias is defined for 'Data'
DESCRIBE(Data, &_::a, &_::b, &_::method)

constexpr auto desc = describe::Get<Data>();
static_assert(desc.all_count == 3);
static_assert(desc.fields_count == 2);
static_assert(desc.methods_count == 1);

void print_fields(const Data& d) {
    desc.for_each_field([&](auto f) {
        std::cout << f.name << ": " << f.get(d) << ", ";
    });
}

void test_print() {
    print_fields(Data{1, 2}); // -> a: 1, b: 2,
}

enum Enum {foo, bar, baz,};
DESCRIBE(Enum, foo, bar, baz) // _:: can be omited

enum class ClEnum {bim, bam, bom,};
DESCRIBE(ClEnum, _::bim, _::bam, _::bom) // enum classes are supported

template<typename T>
void print_enum() {
    describe::Get<T>().for_each_field([&](auto f) {
        using underlying = std::underlying_type_t<typename decltype(f)::type>;
        std::cout << f.name << ": " << underlying(f.value) << ", ";
    });
}

void test() {
    print_enum<Enum>(); //-> foo: 0, bar: 1, baz: 2
    print_enum<ClEnum>(); //-> bim: 0, bam: 1, bom: 2
}
```

## Inheritance

TODO! more info

DESCRIBE_INHERIT(class, parent, ...)

## Attributes

```cpp
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

```

## Templates
```cpp
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
static_assert(desc.name == "Data<T>");
static_assert(desc.ns == "test");
constexpr auto B = desc.get<1>();
static_assert(std::is_same_v<decltype(B)::type, int[10]>);


template<auto a, auto b> struct sum {
    static constexpr auto value = a + b;
    int as_int = int(value);
};

// DESCRIBE_CLASS(...) DESCRIBE_FIELDS(...) pair for multi-param templates
template<auto a, auto b>
DESCRIBE_CLASS(sum<a, b>)
DESCRIBE_FIELDS(&_::as_int)


constexpr auto sum_desc = describe::Get<sum<1, 2>>();
static_assert(sum_desc.name == "sum<a, b>");
static_assert(sum_desc.ns == "");

int main(int argc, char *argv[]) {
    return 0;
}

```

## Private fields
```cpp
#include <describe/describe.hpp>

class Shy {
public:
    ALLOW_DESCRIBE_FOR(Shy)
private:
    int a;
};

DESCRIBE(Shy, &_::a)

class MoreShy {
    int a;
public:
    friend DESCRIBE(MoreShy, &_::a);
};

```
