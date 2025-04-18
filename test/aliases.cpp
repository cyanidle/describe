#include "describe/describe.hpp"
#include <string_view>

enum Mode {
    read,
    write,
    read_write,
};

DESCRIBE("Mode", Mode) {
    MEMBER("r", read);
    MEMBER("read", read);
    MEMBER("w", write);
    MEMBER("write", write);
    MEMBER("rw", read_write);
    MEMBER("read_write", read_write);
}

constexpr Mode convert(std::string_view name) {
    Mode res{};
    if (describe::name_to_enum(name, res)) {
        return res;
    } else {
        throw "Invalid";
    }
}

constexpr auto mode1 = convert("rw");
constexpr auto mode2 = convert("read_write");

static_assert(mode1 == mode2);
static_assert(mode1 == Mode::read_write);