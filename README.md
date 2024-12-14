# describe
Minimal C++17 Reflection Library in 200! LOC. 

# Features
* Constexpr field pointer + name iteration
* Inheritance (NEW: Multiple inheritance)
* Enums support
* Templates support
* Friend support
* Constexpr non-invasive attributes for classes (per-field customizable) (NEW: extract-all)

# Usage

## Basic
```cpp

struct Data {
    int a;
    int b;
    void method() {}
};

DESCRIBE("Data", Data) {
    MEMBER("renamed", &_::a);
    MEMBER("b", &_::b);
    MEMBER("method", &_::method);
}

constexpr auto desc = describe::Get<Data>();

void print_fields(const Data& d) {
    std::cout << desc.name << ": ";
    desc.for_each([&](auto f){
        if constexpr (f.is_field) {
            std::cout << f.name << " -> " << f.get(d) << ", ";
        }
    });
    std::cout << std::endl;
}
```

## Inheritance

```cpp
// Inherit
struct Parent {
    std::string a;
    std::string b;
};

DESCRIBE("Parent", Parent) {
    MEMBER("a", &_::a);
    MEMBER("b", &_::b);
}

struct Child : Parent {
    std::string c;
};


DESCRIBE("Child", Child) {
    PARENT(Parent);
    MEMBER("c", &_::c);
}
```
