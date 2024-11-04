#include <describe/describe.hpp>
#include <iostream>

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

// Inherit
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

// Empty
struct Empty {};
DESCRIBE(Empty)

struct EmptyChild {};
DESCRIBE_INHERIT(Empty, EmptyChild)
