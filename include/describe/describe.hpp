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

template<typename...Ts> struct Attrs {};
template<typename T> struct Tag {using type = T;};
template<auto field> struct Field;
template<typename T> constexpr auto Get();

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

inline constexpr std::string_view next_name(std::string_view& src) {
    auto resStart = src.find("::");
    auto noColons = resStart == std::string_view::npos || src.find_first_of(',') < resStart;
    resStart = noColons ? src.find_first_not_of(' ') : resStart + 2;
    auto resEnd = src.find_first_of(" \t\n\r,", resStart);
    auto result = src.substr(resStart, resEnd - resStart);
    src = src.substr(src.find_first_of(',', resEnd) + 1);
    return result;
}

template<typename T, typename...Ts>
constexpr bool has(Attrs<Ts...>) {
    return (false || ... || std::is_base_of_v<T, Ts>);
}

template<typename T>
auto get_attr(Attrs<>) -> Tag<void> {return {};}

template<typename T, typename Head, typename...Ts>
auto get_attr(Attrs<Head, Ts...>) {
    if constexpr (std::is_base_of_v<T, Head>) {
        return Tag<Head>{};
    } else {
        return detail::get_attr<T>(Attrs<Ts...>{});
    }
}

template<size_t idx, typename Head, typename...Ts> struct pack_idx : pack_idx<idx - 1, Ts...> {};
template<typename Head, typename...Ts> struct pack_idx<0, Head, Ts...> {using type = Head;};

} //detail

template<auto field>
struct Field {
    std::string_view name;
    static constexpr auto value = field;
    using raw_type = decltype(field);
    using type = typename detail::info<raw_type>::type;
    using cls = typename detail::info<raw_type>::cls;
    static constexpr auto is_method = std::is_member_function_pointer_v<raw_type>;
    static constexpr auto is_field = std::is_member_object_pointer_v<raw_type>;
    static constexpr auto is_function = std::is_function_v<raw_type>;
    static constexpr auto is_enum = std::is_enum_v<raw_type>;
    template<typename T> static constexpr decltype(auto) get(T&& obj) noexcept {
        return std::forward<T>(obj).*field;
    }
};

template<typename Cls, typename Parent, typename...Fields>
struct Description : protected Fields...
{
    using type = Cls;
    using parent = Parent;
    std::string_view name;
    static constexpr auto is_enum = std::is_enum_v<Cls>;
    static constexpr auto size = sizeof...(Fields);
    static constexpr auto npos = size_t(-1);
    template<auto member> constexpr auto& cast() {return static_cast<Field<member>&>(*this);}
    template<size_t idx> constexpr auto get() const {
        using f = typename detail::pack_idx<idx, Fields...>::type;
        return static_cast<f>(*this);
    }
    template<typename F> constexpr void for_each(F&& f) const {
        (static_cast<void>(f(static_cast<const Fields&>(*this))), ...);
    }
    template<auto f> static constexpr size_t index_of(Field<f>) noexcept {
        size_t result = npos;
        size_t count = size_t(-1);
        ((count++, std::is_same_v<Field<f>, Fields> && (result = count)), ...);
        return result;
    }
    constexpr size_t index_of(std::string_view name) const noexcept {
        size_t result = npos;
        size_t count = size_t(-1);
        ((count++, (static_cast<const Fields&>(*this).name == name) && (result = count)), ...);
        return result;
    }
    template<auto f> static constexpr size_t index_of() noexcept {
        return index_of<f>(Field<f>{});
    }
};

//! Presence of describe::Tag<ns::T> enables ADL for both describe:: AND ns::
// Main ADL-fallbacks:
void GetDescription(...);
Attrs<> GetAttrs(...);

template<typename C>
struct get_attrs {
    using type = decltype(GetAttrs(Tag<C>{}));
};

template<auto field>
struct get_attrs<Field<field>> {
    // without Tag<Cls> as arg ADL does not work!
    using type = decltype(GetAttrs(Tag<typename Field<field>::cls>{}, Field<field>{}));
};

template<typename Cls, typename...Rest>
struct get_attrs<Description<Cls, Rest...>> : get_attrs<Cls> {};

template<typename Any>
struct get_attrs<const Any> : get_attrs<Any> {};

template<typename T>
using get_attrs_t = typename get_attrs<T>::type;

template<typename T, typename From>
using extract_attr_t = typename decltype(detail::get_attr<T>(get_attrs_t<From>{}))::type;

template<typename T, typename Who>
constexpr bool has_attr_v = detail::has<T>(get_attrs_t<Who>{});

template<typename T>
struct is_described : std::bool_constant<!std::is_void_v<decltype(GetDescription(Tag<T>{}))>> {};

template<typename T>
constexpr auto is_described_v = is_described<T>::value;
template<typename T>
constexpr auto is_described_struct_v = is_described<T>::value && std::is_class_v<T>;
template<typename T>
constexpr auto is_described_enum_v = is_described<T>::value && std::is_enum_v<T>;

template<typename T> constexpr auto Get() {
    constexpr auto res = GetDescription(Tag<T>{});
    return res;
}

template<auto...fields, typename Cls>
constexpr auto Describe(
    Tag<Cls>, std::string_view clsname, std::string_view names)
{
    Description<Cls, void, Field<fields>...> result = {};
    result.name = clsname;
    (static_cast<void>(result.template cast<fields>().name = detail::next_name(names)), ...);
    return result;
}

template<auto...fields, typename Cls, typename ParCls, typename ParCls2, auto...parFields>
constexpr auto Describe(
    Description<ParCls, ParCls2, Field<parFields>...> parent,
    Tag<Cls>, std::string_view clsname, std::string_view names)
{
    Description<Cls, ParCls, Field<parFields>..., Field<fields>...> result = {};
    result.name = clsname;
    (static_cast<void>(result.template cast<parFields>().name = parent.template cast<parFields>().name), ...);
    (static_cast<void>(result.template cast<fields>().name = detail::next_name(names)), ...);
    return result;
}

// Helpers
#define _DESC_STR(...) _DESC_STR2(__VA_ARGS__)
#define _DESC_STR2(...) #__VA_ARGS__
#define _DESC_CAT(x, y) _DESC_CAT_AUX(x, y)
#define _DESC_CAT_AUX(x, y) x##y
// Black magic: use ppstep for these 4
#define _DESC_CONTAINS_COMMA(...) _DESC_X_AS_COMMA(__VA_ARGS__, _DESC_COMMA, ~)
#define _DESC_X_AS_COMMA(_head, x, ...) _DESC_CONTAINS_COMMA_RESULT(x, 0, 1, ~)
#define _DESC_CONTAINS_COMMA_RESULT(x, _, result, ...) result
#define _DESC_COMMA ,
// Macros used to avoid "at least one varg param needed"
#define _DESC_HEAD(head, ...) head
#define _DESC_TAIL(...) _DESC_CAT(_DESC_TAIL, _DESC_CONTAINS_COMMA(__VA_ARGS__))(__VA_ARGS__)
#define _DESC_TAIL0(head)
#define _DESC_TAIL1(head, ...) __VA_ARGS__

#define DESCRIBE_CLASS(...) \
    inline constexpr auto GetDescription(::describe::Tag<__VA_ARGS__>) { \
        using _ = __VA_ARGS__; std::string_view _clsName = _DESC_STR(__VA_ARGS__);

#define DESCRIBE_FIELDS(...) \
    return ::describe::Describe<__VA_ARGS__>(::describe::Tag<_>{}, _clsName, _DESC_STR(__VA_ARGS__));}

#define DESCRIBE_FIELDS_INHERIT(parent, ...) \
    return ::describe::Describe<__VA_ARGS__>(::describe::Get<parent>(), ::describe::Tag<_>{}, _clsName, _DESC_STR(__VA_ARGS__));}

#define DESCRIBE(...) \
    DESCRIBE_CLASS(_DESC_HEAD(__VA_ARGS__, ~)) \
    DESCRIBE_FIELDS(_DESC_TAIL(__VA_ARGS__))

#define DESCRIBE_INHERIT(parent, ...)  \
    DESCRIBE_CLASS(_DESC_HEAD(__VA_ARGS__, ~)) \
    DESCRIBE_FIELDS_INHERIT(parent, _DESC_TAIL(__VA_ARGS__))

//! @achtung @warning THESE Should always be the same for all Translation Units
#define DESCRIBE_ATTRS(cls, ...) \
    auto GetAttrs(::describe::Tag<cls>) -> ::describe::Attrs<__VA_ARGS__>;

//! @achtung @warning THESE Should always be the same for all Translation Units
#define DESCRIBE_FIELD_ATTRS(cls, field, ...) \
    auto GetAttrs(::describe::Tag<cls>, ::describe::Field<&cls::field>) -> ::describe::Attrs<__VA_ARGS__>;

// Utils
template<typename T>
constexpr size_t fields_count() {
    size_t res = 0;
    Get<T>().for_each([&](auto f){
        if constexpr (f.is_field) res++;
    });
    return res;
}

template<typename T>
constexpr auto field_names() {
    std::array<std::string_view, fields_count<T>()> result;
    size_t idx = 0;
    Get<T>().for_each([&](auto f){
        if constexpr (f.is_field) {
            result[idx++] = f.name;
        }
    });
    return result;
}

template<typename Enum, std::enable_if_t<is_described_enum_v<Enum>, int> = 1>
[[nodiscard]]
constexpr bool enum_to_name(Enum value, std::string_view& out) {
    bool found = false;
    Get<Enum>().for_each([&](auto f){
        if constexpr (f.is_enum) {
            if (!found && f.value == value) {
                out = f.name;
                found = true;
            }
        }
    });
    return found;
}

template<typename Enum, std::enable_if_t<is_described_enum_v<Enum>, int> = 1>
[[nodiscard]]
constexpr bool name_to_enum(std::string_view name, Enum& out) {
    bool found = false;
    Get<Enum>().for_each([&](auto f){
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
