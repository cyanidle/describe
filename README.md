# describe
Minimal C++17 header-only Compile-Time Reflection Library in ~300 LOC.

DSCRIBE() + MEMBER() macros allow to create compile-time descriptions for later use in
deserialization, introspection, RPC, meta-objects system and many more

It is simple in API and implementation. Compilation is very fast, considering a very small amount of templates.
Any impact on compilation is lazy (at the place of use)

# Features
* Compile time introspection of classes
* Multiple/Single Inheritance support
* Enumerations support
* Templates support
* Compile time attributes
* Aliases

# Examples
* [Boost.Json](./examples/boost.cpp): Basic integration example
* [nlohmann_json](./examples/nlohmann.cpp): Attributes usage example
* [go_interface](./examples/go_interface.cpp): Advanced meta-magic example
* [LUA Bindings](https://github.com/cyanidle/glua): Auto-generate bindings with LUA
* [RPC](https://github.com/cyanidle/rpcxx): Full-fledged JSON-RPC library with Deserialization based on describe

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
        if constexpr (f.is_method) {
            std::cout << f.name << "()" << ", ";
        }
    });
    std::cout << std::endl;
}

print_fields(Data{1, 2}); // Data: renamed -> 1, b -> 2, method(),
```

## Inheritance

```cpp
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
    PARENT(Parent); // may be repeated with multiple bases
    MEMBER("c", &_::c);
}
```

## Aliases
```cpp
enum Mode {
    read,
    write,
    read_write,
};

DESCRIBE("Mode", Mode) {
    MEMBER("r", read);
    MEMBER("read", read);
    MEMBER("w", write);
    MEMBER("write", write);
    MEMBER("rw", read_write);
    MEMBER("read_write", read_write);
}

constexpr Mode convert(std::string_view name) {
    Mode res{};
    if (describe::name_to_enum(name, res)) {
        return res;
    } else {
        throw "Invalid";
    }
}

constexpr auto mode1 = convert("rw");
constexpr auto mode2 = convert("read_write");

static_assert(mode1 == mode2);
static_assert(mode1 == Mode::read_write);
```