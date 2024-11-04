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
struct Data {
    int a;
    int b;
    void method() {}
};

constexpr auto renamed = &Data::a;
DESCRIBE(Data, renamed, &_::b, &_::method)

constexpr auto desc = describe::Get<Data>();
static_assert(desc.size == 3);

void print_fields(const Data& d) {
    std::cout << desc.name << ": ";
    desc.for_each([&](auto f){
        if constexpr (f.is_field) {
            std::cout << f.name << " -> " << f.get(d) << ", ";
        }
    });
    std::cout << std::endl;
}

enum Enum {foo, bar, baz,};
DESCRIBE(Enum, foo, bar, baz) // _:: can be omited

enum class ClEnum {bim, bam, bom,};
DESCRIBE(ClEnum, _::bim, _::bam, _::bom) // enum classes are supported

template<typename T>
void print_enum(T) {
    constexpr auto desc = describe::Get<T>();
    std::cout << desc.name << ": ";
    desc.for_each([&](auto f) {
        if constexpr (f.is_enum) {
            using underlying = std::underlying_type_t<typename decltype(f)::type>;
            std::cout << f.name << " -> " << underlying(f.value) << ", ";
        }
    });
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    print_fields(Data{1, 2}); // -> renamed: 1, b: 2,
    print_enum(Enum{}); //-> foo: 0, bar: 1, baz: 2
    print_enum(ClEnum{}); //-> bim: 0, bam: 1, bom: 2
}
```

## Inheritance

```cpp
struct Parent {
    std::string a;
    std::string b;
};

DESCRIBE(Parent, &_::a, &_::b)

struct Child : Parent {
    std::string c;
};


DESCRIBE_INHERIT(Parent, Child, &_::c)

constexpr auto child_desc = describe::Get<Child>();
static_assert(child_desc.size == 3);
static_assert(child_desc.get<0>().name == "a");
static_assert(child_desc.get<1>().name == "b");
static_assert(child_desc.get<2>().name == "c");
```

## Attributes

```cpp

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

```

## Templates
```cpp

namespace test {

template<typename T>
struct Data {
    T a;
    T b[10];
};

// all macros work just fine. you just must define template<> above them
template<typename T>
DESCRIBE(test::Data<T>, &_::a, &_::b)

} //test

constexpr auto desc = describe::Get<test::Data<int>>();
constexpr auto B = desc.get<1>();
static_assert(B.name == "b");
static_assert(std::is_same_v<decltype(B)::type, int[10]>);


template<auto a, auto b> struct sum {
    static constexpr auto value = a + b;
    int as_int = int(value);
};

// DESCRIBE_CLASS(...) DESCRIBE_FIELDS(...) pair for multi-param templates
template<auto a, auto b>
DESCRIBE_CLASS(sum<a, b>)
DESCRIBE_FIELDS(&_::as_int) //may be empty!

constexpr auto sum_desc = describe::Get<sum<1, 2>>();
static_assert(sum_desc.name == "sum<a, b>");

```

## Private fields
```cpp

class Shy {
    int a;
public:
    // DESCRIBE() just creates ADL function definition. 
    // we can add `friend`
    friend DESCRIBE(Shy, &_::a);
};

```
