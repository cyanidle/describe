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
    T a18;
    T a19;
    T a20;
    T a21;
    T a22;
    T a23;
    T a24;
    T a25;
    T a26;
    T a27;
    T a28;
    T a29;
};

DESCRIBE_TEMPLATE((typename T), "Test", Test, (T), attr, attr2, attr3) {
    MEMBER("a", &_::a, attr, attr2, attr3);
    MEMBER("a0", &_::a0, attr, attr2, attr3);
    MEMBER("a1", &_::a1, attr, attr2, attr3);
    MEMBER("a2", &_::a2, attr, attr2, attr3);
    MEMBER("a3", &_::a3, attr, attr2, attr3);
    MEMBER("a4", &_::a4, attr, attr2, attr3);
    MEMBER("a5", &_::a5, attr, attr2, attr3);
    MEMBER("a6", &_::a6, attr, attr2, attr3);
    MEMBER("a7", &_::a7, attr, attr2, attr3);
    MEMBER("a8", &_::a8, attr, attr2, attr3);
    MEMBER("a9", &_::a9, attr, attr2, attr3);
    MEMBER("a10", &_::a10, attr, attr2, attr3);
    MEMBER("a11", &_::a11, attr, attr2, attr3);
    MEMBER("a12", &_::a12, attr, attr2, attr3);
    MEMBER("a13", &_::a13, attr, attr2, attr3);
    MEMBER("a14", &_::a14, attr, attr2, attr3);
    MEMBER("a15", &_::a15, attr, attr2, attr3);
    MEMBER("a16", &_::a16, attr, attr2, attr3);
    MEMBER("a17", &_::a17, attr, attr2, attr3);
    MEMBER("a18", &_::a18, attr, attr2, attr3);
    MEMBER("a19", &_::a19, attr, attr2, attr3);
    MEMBER("a20", &_::a20, attr, attr2, attr3);
    MEMBER("a21", &_::a21, attr, attr2, attr3);
    MEMBER("a22", &_::a22, attr, attr2, attr3);
    MEMBER("a23", &_::a23, attr, attr2, attr3);
    MEMBER("a24", &_::a24, attr, attr2, attr3);
    MEMBER("a25", &_::a25, attr, attr2, attr3);
    MEMBER("a26", &_::a26, attr, attr2, attr3);
    MEMBER("a27", &_::a27, attr, attr2, attr3);
    MEMBER("a28", &_::a28, attr, attr2, attr3);
    MEMBER("a29", &_::a29, attr, attr2, attr3);
}

template<typename T>
void test_single() {
    using Current = Test<T>;
    Current obj;
    using cls_a = describe::extract_t<_attr, Current>;
    using cls_b = describe::extract_t<_attr2, Current>;
    using cls_c = describe::extract_t<_attr3, Current>;
    using cls_d = describe::extract_t<_attr4, Current>;
    using cls_all_a = describe::extract_all_t<_attr, Current>;
    using cls_all_b = describe::extract_all_t<_attr2, Current>;
    using cls_all_c = describe::extract_all_t<_attr3, Current>;
    using cls_all_d = describe::extract_all_t<_attr4, Current>;
    describe::Get<Current>::for_each([&](auto f){
        using F = decltype(f);
        using a = describe::extract_t<_attr, F>;
        using b = describe::extract_t<_attr2, F>;
        using c = describe::extract_t<_attr3, F>;
        using d = describe::extract_t<_attr4, F>;
        using all_a = describe::extract_all_t<_attr, F>;
        using all_b = describe::extract_all_t<_attr2, F>;
        using all_c = describe::extract_all_t<_attr3, F>;
        using all_d = describe::extract_all_t<_attr4, F>;
        f.get(obj);
    });
}

template<typename...Ts>
void test_all() {
    (test_single<Ts>(), ...);
}

void test() {
    test_all<int, bool, char, char16_t, char32_t, attr, attr2, attr3, float, double>();
}
