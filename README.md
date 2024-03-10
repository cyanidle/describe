# describe
Minimal C++ Reflection Library in just ~160 LOC

# Usage

```cpp
#include<describe/describe.hpp>

struct Data {
    int a;
    int b;
    void method() {

    }
};

DESCRIBE(Data, &_::a, &_::b, &_::method)

constexpr auto desc = describe::Get<Data>();
static_assert(desc.all_count == 3);
static_assert(desc.fields_count == 2);
static_assert(desc.methods_count == 1);

void print_fields(const Data& d) {
    desc.for_each_field([&](auto f) {
        using type = typename decltype(f)::type;
        std::cout << f.name << ": "
                  << d.*f.value
                  << ", ";
    });
}

print_fields(Data{1, 2}); // -> a: 1, b: 2, 
```
# Advanced

It is possible to describe namespaces for classes AND meta info for fields/methods

```cpp
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
//       ^ must write namespace        ^ everything is captured Literally (without macro expansion)
//                     ^ '&' and '::' are discarded

// NOTE: '::' is required before field name!
// Fields are separated based on commas! do not put commas inside meta data!

}

void example() {
    a::My obj;
    constexpr auto desc = describe::Get<a::My>();
    // ^ this description can be used in compile-time functions!
    constexpr auto a = desc.get<0>();

    static_assert(std::is_same_v<decltype(a), const describe::Field<&a::My::a>>);

    constexpr auto b = desc.get<1>(); //these are in the same order as in DESCRIBE()
    constexpr auto c = desc.get<2>();
    constexpr auto d = desc.get<3>();
    static_assert(desc.fields_count == 4);
    static_assert(desc.name == "My");
    static_assert(desc.meta == "a");

    static_assert(a.name == "a");
    obj.*a.value = 33; // write value
    a.get(obj) = 42; // alternative syntax to read/write
    static_assert(a.meta == "_");

    static_assert(b.name == "b");
    static_assert(b.meta == "optional");

    static_assert(c.name == "c");
    static_assert(c.meta == "one_of(foo|bar|baz)");
    using c_type = decltype(of(c)); // ADL-powered helper to get type of field;
    static_assert(std::is_same_v<c_type, std::string>);
    using owner_type = decltype(class_of(c));
    static_assert(std::is_same_v<owner_type, a::My>);
    c_type current_value = obj.*c.value;

    static_assert(d.name == "d");
    static_assert(d.meta == "in_range(0 < x < 15)");
}

// Templates and privates work too!

template<typename T>
struct A {
    T data;
};

template<typename T>
DESCRIBE(A<T>, &_::data)

constexpr auto templ_data = describe::Get<A<int>>().get<0>();
static_assert(templ_data.name == "data");
static_assert(std::is_same_v<int, decltype(of(templ_data))>);

struct B {
    friend DESCRIBE(B, &_::priv_data)
private:
    int priv_data;
};


constexpr auto priv_data = describe::Get<B>().get<0>();
static_assert(priv_data.name == "priv_data");
```
