/*
describe.hpp

MIT License

Copyright (c) 2024 Доронин Алексей

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef DESCRIBE_HPP
#define DESCRIBE_HPP
#include <stdint.h>
#include <stddef.h>
#include <string_view>
#include <type_traits>
#include <utility>
#include <array>
namespace describe
{

template<typename...Ts> struct TypeList {
    static constexpr size_t size = sizeof...(Ts);
};

template<typename T> struct Tag {
    using type = T;
};

namespace detail {

template<typename T, typename = void>
struct info {
    using cls = void;
    using type = T;
};

template <class C, typename T> auto get_memptr_type(T C::*) -> Tag<T>;
template <class C, typename T> auto get_memptr_class(T C::*) -> Tag<C>;
template <class C, typename T, typename...Args> auto get_memptr_type(T (C::*v)(Args...)) -> Tag<decltype(v)>;
template <class C, typename T, typename...Args> auto get_memptr_class(T (C::*)(Args...)) -> Tag<C>;

template<typename T>
struct info<T, std::enable_if_t<std::is_member_pointer_v<T>>> {
    using cls = typename decltype(detail::get_memptr_class(T{}))::type;
    using type = typename decltype(detail::get_memptr_type(T{}))::type;
};

} //detail

template<auto field, typename...Attrs>
struct Member {
    const char* name;
    static constexpr auto value = field;
    using raw_type = decltype(field);
    using type = typename detail::info<raw_type>::type;
    using cls = typename detail::info<raw_type>::cls;
    static constexpr auto is_method = std::is_member_function_pointer_v<raw_type>;
    static constexpr auto is_field = std::is_member_object_pointer_v<raw_type>;
    static constexpr auto is_function = std::is_function_v<raw_type>;
    static constexpr auto is_enum = std::is_enum_v<raw_type>;
    using Attributes = TypeList<Attrs...>;
    template<typename T> static constexpr decltype(auto) get(T&& obj) noexcept {
        return std::forward<T>(obj).*field;
    }
};

// If you get error 'no _OPENVA' -> you forgot to pass params in parens like this: '(x, y, z)'
#define _OPENVA(...) __VA_ARGS__
#define _PPCAT(x, y) _PPCAT2(x, y)
#define _PPCAT2(x, y) x##y

// fallback to make Get<T> return void if T is not described
auto DescribeHelper(...) -> void;

#define DO_DESCRIBE(templ, names, helper, cls, ...) \
templ struct helper { \
    using _ = cls names;  \
    static constexpr const char* name = #cls; \
    using Attrs = describe::TypeList<__VA_ARGS__>; \
    template<typename Fn> static constexpr void for_each(Fn _desc); \
};\
templ auto DescribeHelper(describe::Tag<cls names>) -> helper names; \
templ template<typename Fn> \
constexpr void helper names::for_each(Fn _desc)

#define DESCRIBE_TEMPLATE(params, cls, names, ...) \
DO_DESCRIBE(template<_OPENVA params>, <_OPENVA names>, _PPCAT(cls, _Describe), cls, __VA_ARGS__)

#define DESCRIBE(cls, ...) \
DO_DESCRIBE(,,_PPCAT(cls, _Describe), cls, __VA_ARGS__)

#define PARENT(...) describe::Get<__VA_ARGS__>::for_each(_desc)
#define MEMBER(name, x, ...) _desc(describe::Member<x, ##__VA_ARGS__>{name})

template<typename T>
using Get = decltype(DescribeHelper(Tag<T>{}));

// Utils
template<typename T>
constexpr size_t fields_count() {
    size_t res = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_field) res++;
    });
    return res;
}

template<typename T>
constexpr auto field_names() {
    std::array<const char*, fields_count<T>()> result;
    size_t idx = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_field) {
            result[idx++] = f.name;
        }
    });
    return result;
}

template<typename Enum>
[[nodiscard]]
constexpr bool enum_to_name(Enum value, std::string_view& out) {
    bool found = false;
    Get<Enum>::for_each([&](auto f){
        if constexpr (f.is_enum) {
            if (!found && f.value == value) {
                out = f.name;
                found = true;
            }
        }
    });
    return found;
}

template<typename Enum>
[[nodiscard]]
constexpr bool name_to_enum(std::string_view name, Enum& out) {
    bool found = false;
    Get<Enum>::for_each([&](auto f){
        if constexpr (f.is_enum) {
            if (!found && f.name == name) {
                out = f.value;
                found = true;
            }
        }
    });
    return found;
}

} //describe

#endif //DESCRIBE_HPP
