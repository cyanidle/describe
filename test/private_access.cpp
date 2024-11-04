#include <describe/describe.hpp>

class Shy {
    int a;
public:
    friend DESCRIBE(Shy, &_::a);
};
