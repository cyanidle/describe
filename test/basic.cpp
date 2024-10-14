#include <describe/describe.hpp>
#include <iostream>

struct Data {
    int a;
    int b;
    void method() {

    }
};

constexpr auto r = &Data::a; //rename field
DESCRIBE(Data, r, &_::b, &_::method)

constexpr auto desc = describe::Get<Data>();
static_assert(desc.all_count == 3);
static_assert(desc.fields_count == 2);
static_assert(desc.methods_count == 1);

void print_fields(const Data& d) {
    std::cout << desc.name << ": ";
    desc.for_each_field([&](auto f) {
        std::cout << f.name << " -> " << f.get(d) << ", ";
    });
    std::cout << std::endl;
}

enum Enum {foo, bar, baz,};
DESCRIBE(Enum, foo, bar, baz) // _:: can be omited

enum class ClEnum {bim, bam, bom,};
DESCRIBE(ClEnum, _::bim, _::bam, _::bom) // enum classes are supported

template<typename T>
void print_enum() {
    constexpr auto desc = describe::Get<T>();
    std::cout << desc.name << ": ";
    desc.for_each_field([&](auto f) {
        using underlying = std::underlying_type_t<typename decltype(f)::type>;
        std::cout << f.name << " -> " << underlying(f.value) << ", ";
    });
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    print_fields(Data{1, 2}); // -> a: 1, b: 2,
    print_enum<Enum>(); //-> foo: 0, bar: 1, baz: 2
    print_enum<ClEnum>(); //-> bim: 0, bam: 1, bom: 2
}
