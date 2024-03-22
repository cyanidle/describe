#include <describe/describe.hpp>

class Shy {
public:
    ALLOW_DESCRIBE_FOR(Shy)
private:
    int a;
};

DESCRIBE(Shy, &_::a)

class MoreShy {
    int a;
public:
    friend DESCRIBE(MoreShy, &_::a);
};

int main(int argc, char *argv[])
{

    return 0;
}
