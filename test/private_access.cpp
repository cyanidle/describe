#include <describe/describe.hpp>

class Shy {
    int a;
public:
    friend class Shy_Describe;
};

DESCRIBE(Shy) {
    MEMBER("a", &_::a);
}
