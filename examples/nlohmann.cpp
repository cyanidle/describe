#include <describe/describe.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

// Implementation may define attributes

struct InRangeBase {}; // used to query for derived InRange<x, y>
struct ValidatedBase {};  // used to query for derived Validated<func>

// Annotate enum class to make serialize it as Underlying type integer
struct EnumAsInteger {};

// String fields annotated with this strip '?' from input and write '?' on serialize
struct WithQuestionMark {};

// Validate that integer is in range on deserialize
template<int min, int max>
struct InRange : InRangeBase {
    static constexpr int Min = min;
    static constexpr int Max = max;
};

// Call a function on annotated class or field after deserialize
template<auto validator>
struct Validated : ValidatedBase {
    template<typename T>
    static void validate(T& object) {
        validator(object);
    }
};

/// Actual implementation for ALL described object

template<typename T>
struct nlohmann::adl_serializer<T, describe::if_described_enum_t<T>>
{
    template<typename BasicJsonType, typename TargetType = T>
    static void from_json(BasicJsonType && j, TargetType& val)
    {
        bool hit = false; // to allow aliases for values of enum. First one used
        if constexpr (describe::has_v<EnumAsInteger, T>) {
            auto i = j.template get<std::underlying_type_t<T>>();
            describe::Get<T>::for_each([&](auto info){
                if (!hit && i == info.value) {
                    val = info.value;
                    hit = true;
                }
            });
        } else {
            auto str = j.template get<std::string>();
            describe::Get<T>::for_each([&](auto info){
                if (!hit && str == info.name) {
                    val = info.value;
                    hit = true;
                }
            });
        }
        if (!hit) {
            throw std::runtime_error(
                "Could not deserialize enum: " + std::string{describe::Get<T>::name}
            );
        }
    }

    template<typename BasicJsonType, typename TargetType = T>
    static T from_json(BasicJsonType && j)
    {
        T res;
        from_json(j, res);
        return res;
    }

    template<typename BasicJsonType, typename TargetType = T>
    static void to_json(BasicJsonType& j, TargetType && val)
    {
        bool hit = false; // to allow aliases for values of enum. First one used
        describe::Get<T>::for_each([&](auto info){
            if (!hit && val == info.value) {
                if constexpr (describe::has_v<EnumAsInteger, T>) {
                    j = std::underlying_type_t<T>(val);
                } else {
                    j = info.name;
                }
                hit = true;
            }
        });
    }
};

static void check_range(std::string_view name, int val, int min, int max) {
    using namespace std;
    if (val < min || val > max) {
        throw runtime_error(
            string{name} +
            ": value: (" + to_string(val) +
            ") => not in range: [" + to_string(min) + "-" + to_string(max) + "]");
    }
}

template<typename T>
struct nlohmann::adl_serializer<T, describe::if_described_struct_t<T>>
{
    template<typename BasicJsonType, typename TargetType = T>
    static void from_json(BasicJsonType && j, TargetType& val)
    {
        describe::Get<T>::for_each([&](auto info){
            using Field = decltype(info);
            if constexpr (info.is_field) {
                auto& field = info.get(val);
                field = j[info.name].template get<decltype(of(info))>();
                if constexpr (describe::has_v<WithQuestionMark, Field>) {
                    if (field.size() && field.back() == '?') {
                        field.pop_back();
                    }
                }
                using range = describe::extract_t<InRangeBase, Field>;
                if constexpr (!std::is_void_v<range>) {
                    check_range(info.name, field, range::Min, range::Max);
                }
                using validator = describe::extract_t<ValidatedBase, Field>;
                if constexpr (!std::is_void_v<validator>) {
                    validator::validate(field);
                }
            }
        });
        using validator = describe::extract_t<ValidatedBase, T>;
        if constexpr (!std::is_void_v<validator>) {
            validator::validate(val);
        }
    }

    template<typename BasicJsonType, typename TargetType = T>
    static T from_json(BasicJsonType && j)
    {
        T res;
        from_json(j, res);
        return res;
    }

    template<typename BasicJsonType, typename TargetType = T>
    static void to_json(BasicJsonType& j, TargetType && val)
    {
        j = json::object();
        describe::Get<T>::for_each([&](auto info){
            using Field = decltype(info);
            if constexpr (info.is_field) {
                auto& field = info.get(val);
                if constexpr (describe::has_v<WithQuestionMark, Field>) {
                    j[info.name] = json(field + "?");
                } else {
                    j[info.name] = json(field);
                }
            }
        });
    }
};


/////////// Usage:


enum ObjectType {
    Good,
    Bad,
    Ugly,
};

DESCRIBE("ObjectType", ObjectType) {
    MEMBER("good", Good);
    MEMBER("bad", Bad);
    MEMBER("ugly", Ugly); // serialized into this if == ObjectType::Ugly
    MEMBER("UGLY", Ugly); // "for backwards-compat" (can be deserialized from this string)
    // You can make a custom attribute for case-insensitive deserialize (like with AsInteger)
}

enum SizeHint {
    Small = 3,
    Medium = 6,
    Big = 9,
};

DESCRIBE("SizeHint", SizeHint, EnumAsInteger) {
    MEMBER("small", Small);
    MEMBER("medium", Medium);
    MEMBER("big", Big);
}

using namespace std;
using namespace nlohmann;

struct Object {
    int number;
    string name;
    ObjectType type;
    SizeHint hint;

    static void validate(Object& self) {
        if (self.type == Ugly && self.hint == Big) {
            throw runtime_error("big AND ugly!!!");
        }
    }
};


DESCRIBE("Object", Object, Validated<Object::validate>) {
    MEMBER("number", &_::number, InRange<0, 7>);
    MEMBER("name", &_::name, WithQuestionMark);
    MEMBER("type", &_::type);
    MEMBER("hint", &_::hint);
}

int main(int argc, char *argv[])
{
    auto string = json(Object{1, "john", Ugly, Medium}).dump(2);
    std::cout << string << std::endl << std::endl;

    auto back = json::parse(string).get<Object>();

    std::cout << "Round trip: " << std::endl;
    std::cout << json(back).dump(2) << std::endl << std::endl;

    return 0;
}
