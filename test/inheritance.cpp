#include <describe/describe.hpp>
#include <string>

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