# describe
Minimal C++ Reflection Library


# Usage

```cpp
#include<describe.hpp>
struct Data {   
    int a;
    int b;
    void method() {
    
    }
}
DESCRIBE(Data, &_::a, &_::b, &_::method)

constexpr auto desc = decribe::Get<Data>();
static_assert(desc.all_count == 3)
static_assert(desc.fields_count == 2)
static_assert(desc.methods_count == 1)

void print_fields(const Data& d) {  
    desc.for_each_field([&](auto f) {   
        using type = typename decltype(f)::type;
        std::cout << f.name << ": " 
                  << d.*f.value
                  << ", ";
    });
}

print_fields(Data{1, 2}); // -> "a: 1, b: 2,"

```

