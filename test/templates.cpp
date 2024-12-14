#include <describe/describe.hpp>

namespace test {

template<typename T, typename U>
struct Data {
    T a;
    U b[10];
};

DESCRIBE_TEMPLATE((typename T, typename U), "test::Data", Data, (T, U)) {
    MEMBER("a", &_::a);
    MEMBER("b", &_::b);
}

} //test
