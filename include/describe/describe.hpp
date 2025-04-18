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
    std::string_view name;
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
        static_assert(is_field);
        return std::forward<T>(obj).*field;
    }
    template<typename Object, typename...Args> static constexpr decltype(auto) call(Object&& object, Args&&...args) noexcept {
        static_assert(is_method);
        return (std::forward<Object>(object).*value)(std::forward<Args>(args)...);
    }
    static constexpr Attributes attrs() {return {};}
};

template<auto f, typename...A>
auto of(Member<f, A...> mem) -> typename decltype(mem)::type;

template<auto f, typename...A>
auto cls(Member<f, A...> mem) -> typename decltype(mem)::cls;

template<auto f, typename...A>
auto attrs(Member<f, A...> mem) -> TypeList<A...>;

// If you get error 'no _OPENVA' -> you forgot to pass params in parens like this: '(x, y, z)'
#define _OPENVA(...) __VA_ARGS__
#define _PPCAT(x, y) _PPCAT2(x, y)
#define _PPCAT2(x, y) x##y

// fallback to make Get<T> return void if T is not described
auto DescribeHelper(...) -> void;

#define DO_DESCRIBE(templ, use_templ, helper, _name, cls, ...) \
templ struct helper { \
    using _ = cls use_templ;  \
    [[maybe_unused]] static constexpr std::string_view name = _name; \
    using Attributes = describe::TypeList<__VA_ARGS__>; \
    static constexpr Attributes attrs() {return {};} \
    template<typename Fn> static constexpr void for_each(Fn _desc); \
};\
templ [[maybe_unused]] auto DescribeHelper(describe::Tag<cls use_templ>) -> helper use_templ; \
templ template<typename Fn> \
constexpr void helper use_templ::for_each([[maybe_unused]] Fn _desc)

#define DESCRIBE_TEMPLATE(templ, name, cls, use_templ, ...) \
DO_DESCRIBE(template<_OPENVA templ>, <_OPENVA use_templ>, _PPCAT(cls, _Describe), name, cls, __VA_ARGS__)

#define DESCRIBE(name, cls, ...) \
DO_DESCRIBE(,,_PPCAT(cls, _Describe), name, cls, __VA_ARGS__)

#define PARENT(...) describe::Get<__VA_ARGS__>::for_each(_desc)
#define MEMBER(name, x, ...) _desc(describe::Member<x, ##__VA_ARGS__>{name})

template<typename T>
using Get = decltype(DescribeHelper(Tag<T>{}));

template<typename T>
constexpr bool is_described_v = !std::is_void_v<Get<T>>;

template<typename T, typename R = void>
using if_described_t = std::enable_if_t<is_described_v<T>, R>;

template<typename T>
constexpr bool is_described_struct_v = is_described_v<T> && std::is_class_v<T>;

template<typename T, typename R = void>
using if_described_struct_t = std::enable_if_t<is_described_struct_v<T>, R>;

template<typename T>
constexpr bool is_described_enum_v = is_described_v<T> && std::is_enum_v<T>;

template<typename T, typename R = void>
using if_described_enum_t = std::enable_if_t<is_described_enum_v<T>, R>;

namespace detail {

template<typename T, typename...A>
auto has(TypeList<A...>) -> std::bool_constant<(false || ... || std::is_base_of_v<T, A>)>;

template<typename T>
auto extract(TypeList<>) -> Tag<void>;
template<typename T, typename H, typename...Tail>
auto extract(TypeList<H, Tail...>) {
    if constexpr (std::is_base_of_v<T, H>) return Tag<H>{};
    else return extract<T>(TypeList<Tail...>{});
}

template<typename...Ts>
struct merge;

template<typename...L, typename...R, typename...Tail>
struct merge<TypeList<L...>, TypeList<R...>, Tail...>
{
    using type = typename merge<TypeList<L..., R...>, Tail...>::type;
};

template<typename T>
struct merge<T>
{
    using type = T;
};

template<>
struct merge<>
{
    using type = TypeList<>;
};

template<typename T, typename...A>
auto extract_all(TypeList<A...>)
    -> typename merge<std::conditional_t<std::is_base_of_v<T, A>, TypeList<A>, TypeList<>>...>::type;

} //detail

template<typename T>
struct get_attrs {
    using type = typename Get<T>::Attributes;
};

template<typename...A>
struct get_attrs<TypeList<A...>> {
    using type = TypeList<A...>;
};

template<typename T>
using get_attrs_t = typename get_attrs<T>::type;

template<typename T, typename From>
using extract_t = typename decltype(detail::extract<T>(get_attrs_t<From>{}))::type;

template<typename T, typename From>
using extract_all_t = decltype(detail::extract_all<T>(get_attrs_t<From>{}));

template<typename T, typename Who>
constexpr bool has_v = decltype(detail::has<T>(get_attrs_t<Who>{}))::value;

// Utils
template<typename T, if_described_struct_t<T, int> = 1>
constexpr size_t fields_count() {
    size_t res = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_field) res++;
    });
    return res;
}

template<typename T, if_described_struct_t<T, int> = 1>
constexpr auto field_names() {
    std::array<std::string_view, fields_count<T>()> result;
    size_t idx = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_field) result[idx++] = f.name;
    });
    return result;
}

template<typename T, if_described_enum_t<T, int> = 1>
constexpr size_t enums_count() {
    size_t res = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_enum) res++;
    });
    return res;
}

template<typename T, if_described_enum_t<T, int> = 1>
constexpr auto enum_names() {
    std::array<std::string_view, enums_count<T>()> result;
    size_t idx = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_enum) result[idx++] = f.name;
    });
    return result;
}

template<typename T, if_described_struct_t<T, int> = 1>
constexpr size_t methods_count() {
    size_t res = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_method) res++;
    });
    return res;
}

template<typename T, if_described_struct_t<T, int> = 1>
constexpr auto method_names() {
    std::array<std::string_view, methods_count<T>()> result;
    size_t idx = 0;
    Get<T>::for_each([&](auto f){
        if constexpr (f.is_method) result[idx++] = f.name;
    });
    return result;
}

template<typename Enum, if_described_enum_t<Enum, int> = 1>
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

template<typename Enum, if_described_enum_t<Enum, int> = 1>
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