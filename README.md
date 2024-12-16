# describe
Minimal C++17 Reflection Library in 200! LOC. 

# Features
* Constexpr field pointer + name iteration
* Inheritance (NEW: Multiple inheritance)
* Enums support
* Templates support
* Friend (private variables) support
* Constexpr nonattributes for classes AND fields (NEW: extract-all)

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

template<typename T>
void print_fields(const T& d) {
    constexpr auto desc = describe::Get<T>();
    std::cout << desc.name << ": ";
    desc.for_each([&](auto f){
        if constexpr (f.is_field) {
            std::cout << f.name << " -> " << f.get(d) << ", ";
        }
    });
    std::cout << std::endl;
}

print_fields(Data{1, 2}); // Data: rename -> 1, b -> 2, 
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
