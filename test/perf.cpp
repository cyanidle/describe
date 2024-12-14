#include "describe/describe.hpp"

struct _attr{};
struct _attr2{};
struct _attr3{};
struct _attr4{};

struct attr : _attr {};
struct attr2 : _attr {};
struct attr3 : _attr {};

template<typename T>
struct Test {
    T a;
    T a0;
    T a1;
    T a2;
    T a3;
    T a4;
    T a5;
    T a6;
    T a7;
    T a8;
    T a9;
    T a10;
    T a11;
    T a12;
    T a13;
    T a14;
    T a15;
    T a16;
    T a17;
};

template<typename T>
DESCRIBE(Test<T>, &_::a, &_::a0, &_::a1, &_::a2,
         &_::a3, &_::a4, &_::a5, &_::a6, &_::a7,
         &_::a8, &_::a9, &_::a10, &_::a11, &_::a12,
         &_::a13, &_::a14, &_::a15, &_::a16, &_::a17)
template<typename T>
DESCRIBE_ATTRS(Test<T>, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a0, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a1, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a2, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a3, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a4, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a5, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a6, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a7, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a8, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a9, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a10, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a11, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a12, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a13, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a14, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a15, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a16, attr, attr2, attr3)
template<typename T> DESCRIBE_FIELD_ATTRS(Test<T>, a17, attr, attr2, attr3)

template<typename T>
void test_single() {
    using Current = Test<T>;
    Current obj;
    using cls_a = describe::extract_attr_t<_attr, Current>;
    using cls_b = describe::extract_attr_t<_attr2, Current>;
    using cls_c = describe::extract_attr_t<_attr3, Current>;
    using cls_d = describe::extract_attr_t<_attr4, Current>;
    describe::Get<Current>().for_each([&](auto f){
        using F = decltype(f);
        using a = describe::extract_attr_t<_attr, F>;
        using b = describe::extract_attr_t<_attr2, F>;
        using c = describe::extract_attr_t<_attr3, F>;
        using d = describe::extract_attr_t<_attr4, F>;
        f.get(obj);
    });
}

template<typename...Ts>
void test_all() {
    (test_single<Ts>(), ...);
}

void test() {
    test_all<int, bool, char, char16_t, char32_t, attr, attr2, attr3>();
}
