# describe
Minimal C++ Reflection Library


# Usage

```cpp
#include<describe/describe.hpp>
struct Data {   
    int a;
    int b;
    void method() {
    
    }
}
DESCRIBE(Data, &_::a, &_::b, &_::method)

constexpr auto desc = decribe::Get<Data>();
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

print_fields(Data{1, 2}); // -> "a: 1, b: 2, "

```

# Advanced

It is possible to describe namespaces for classes AND meta info for fields/methods

```cpp
namespace a {

// defining a meta-info tag for fields
#define optional _

struct My {
    int a, b;
};

// inside this macro '_' is actually an alias to a::My
DESCRIBE(a::My, &_::a, &optional::b)
//       ^ namespace        ^ everything is captured Literally (without macro expansion)
//                     ^ '&' and '::' are discarded

}

void test() {
    auto desc = describe::Get<a::My>();
    auto a = desc.get<0>();
    auto b = desc.get<1>(); //these are in the same order as in DESCRIBE()
    desc.cls_name == "My"; // true
    desc.ns_name == "a"; // true
    
    a.name == "a"; // true
    a.meta == "_"; // true
    
    b.name == "b"; // true
    b.meta == "optional"; // true
}
```
