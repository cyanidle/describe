#include <describe/describe.hpp>
#include <iostream>
#include <type_traits>

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

        using cls = decltype(cls(f));
        static_assert(std::is_same_v<cls, Data>);

        if constexpr (f.is_field) {
            std::cout << f.name << " -> " << f.get(d) << ", ";
        }
    });
    std::cout << std::endl;
}

enum Enum {foo, bar, baz,};
DESCRIBE("Enum", Enum) {
    MEMBER("foo", foo);
    MEMBER("bar", bar);
    MEMBER("baz", baz);
}

enum class ClEnum {bim, bam, bom,};
DESCRIBE("ClEnum", ClEnum) {
    MEMBER("bim", _::bim);
    MEMBER("bam", _::bam);
    MEMBER("bom", _::bom);
}

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

    Enum value = foo;
    std::string_view value_name;
    if (!describe::enum_to_name(value, value_name) || value_name != "foo") {
        return 1;
    }
    if (!describe::name_to_enum(value_name, value) || value != foo) {
        return 1;
    }
    if (describe::name_to_enum("asd", value)) {
        return 1;
    }
    return 0;
}

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
