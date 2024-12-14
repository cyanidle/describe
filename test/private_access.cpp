#include <describe/describe.hpp>

class Shy {
    int a;
public:
    friend class Shy_Describe;
};

DESCRIBE("Shy", Shy) {
    MEMBER("a", &_::a);
}
