#ifndef DESCRIBE_HPP
#define DESCRIBE_HPP
#include <stdint.h>
#include <stddef.h>
#include <string_view>
#include <type_traits>
#include <utility>
#include <typeindex>
#include <array>
namespace describe
{

template<typename...Ts> struct Attrs {
    using GetAttrs = Attrs;
};
template<typename T> struct Tag {using type = T;};
template<auto field> struct Field;
template<typename T> constexpr auto Get();

namespace detail {

template <typename T, std::enable_if_t<std::is_enum_v<T>,int> = 0> auto get_memptr_type(T) -> Tag<T>;
template <typename T, std::enable_if_t<std::is_enum_v<T>,int> = 0> auto get_memptr_class(T) -> Tag<T>;
template <class C, typename T> auto get_memptr_type(T C::*) -> Tag<T>;
template <class C, typename T> auto get_memptr_class(T C::*) -> Tag<C>;
template <class C, typename T, typename...Args>
auto get_memptr_type(T (C::*v)(Args...)) -> Tag<decltype(v)>;
template <class C, typename T, typename...Args>
auto get_memptr_class(T (C::*)(Args...)) -> Tag<C>;

template<bool methods, typename...T>
constexpr size_t count() {
    return ((1 * (methods == T::is_method)) + ... + 0);
}
constexpr std::string_view next_name(std::string_view& src) {
    auto resStart = src.find("::");
    resStart = resStart == std::string_view::npos ? src.find_first_not_of(" ") : resStart + 2;
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
        return get_attr<T>(Attrs<Ts...>{});
    }
}

template<typename T, typename=void> struct has_attrs : std::false_type {};
template<typename T, typename=void> struct has_field_attrs : std::false_type {};
template<typename T, typename=void> struct has_static_attrs : std::false_type {};
template<typename T> struct has_attrs<T, std::void_t<decltype(GetAttrs(Tag<T>{}))>> : std::true_type {};
template<typename T> struct has_field_attrs<T, std::void_t<decltype(GetAttrs(Tag<typename T::cls>{}, T{}))>> : std::true_type {};
template<typename T> struct has_static_attrs<T, std::void_t<typename T::GetAttrs>> : std::true_type {};
template<size_t idx, typename Head, typename...Ts> struct pack_idx : pack_idx<idx - 1, Ts...> {};
template<typename Head, typename...Ts> struct pack_idx<0, Head, Ts...> {using type = Head;};
} //detail

template<auto field> struct Field {
    static_assert(std::is_member_pointer_v<decltype(field)> || std::is_enum_v<decltype(field)>,
        "Field can only be used with &_::members or enums");
    std::string_view name;
    static constexpr auto value = field;
    using type = typename decltype(detail::get_memptr_type(field))::type;
    static constexpr auto is_method = std::is_member_function_pointer_v<type>;
    static constexpr auto is_enum = std::is_enum_v<type>;
    using cls = typename decltype(detail::get_memptr_class(field))::type;
    template<typename T> static constexpr auto&& get(T&& obj) noexcept {
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
    static constexpr auto fields_count = detail::count<false, Fields...>();
    static constexpr auto methods_count = detail::count<true, Fields...>();
    static constexpr auto all_count = fields_count + methods_count;
    static constexpr auto npos = size_t(-1);
    template<auto member> constexpr auto& get() {return static_cast<Field<member>&>(*this);}
    template<auto member> constexpr const auto& get() const {return static_cast<const Field<member>&>(*this);}
    template<typename F> constexpr void for_each_field(F&& f) const {
        auto helper = [&f](auto field){
            if constexpr (field.is_method == false) f(field);
        };
        (helper(static_cast<const Fields&>(*this)), ...);
    }
    template<typename F> constexpr void for_each_method(F&& f) const {
        auto helper = [&f](auto field){
            if constexpr (field.is_method == true) f(field);
        };
        (helper(static_cast<const Fields&>(*this)), ...);
    }
    template<typename F> constexpr void for_each(F&& f) const {
        (static_cast<void>(f(static_cast<const Fields&>(*this))), ...);
    }
    template<auto f> static constexpr size_t index_of(Field<f>) {
        size_t result = npos;
        size_t count = size_t(-1);
        ((count++, std::is_same_v<Field<f>, Fields> && (result = count)), ...);
        return result;
    }
    template<auto f> static constexpr size_t index_of() {
        return index_of<f>(Field<f>{});
    }
    constexpr size_t index_of(std::string_view name) const {
        size_t result = npos;
        size_t count = size_t(-1);
        ((count++, (static_cast<const Fields&>(*this).name == name) && (result = count)), ...);
        return result;
    }
};

template<size_t idx, typename Cls, typename Parent, typename...Fields>
constexpr auto& by_index(Description<Cls, Parent, Fields...>& desc) {
    using f = typename detail::pack_idx<idx, Fields...>::type;
    return desc.template get<f::value>();
}

template<size_t idx, typename Cls, typename Parent, typename...Fields>
constexpr const auto& by_index(Description<Cls, Parent, Fields...> const& desc) {
    using f = typename detail::pack_idx<idx, Fields...>::type;
    return desc.template get<f::value>();
}

template<typename F, typename = void>
struct get_attrs {
    using type = Attrs<>;
};

template<typename C>
struct get_attrs<C, std::enable_if_t<detail::has_static_attrs<C>::value>> {
    using type = typename C::GetAttrs;
};

template<typename C>
struct get_attrs<C, std::enable_if_t<detail::has_attrs<C>::value>> {
    using type = decltype(GetAttrs(Tag<C>{}));
};

template<auto field>
struct get_attrs<Field<field>, std::enable_if_t<detail::has_field_attrs<Field<field>>::value>> {
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

template<typename T, typename=void>
struct is_described : std::false_type {};

template<typename T>
struct is_described<T,
    std::void_t<decltype(GetDescription(Tag<T>{}))
>>: std::true_type {};

template<typename T>
constexpr auto is_described_v = is_described<T>::value;
template<typename T>
constexpr auto is_described_struct_v = is_described<T>::value && !std::is_enum_v<T>;
template<typename T>
constexpr auto is_described_enum_v = is_described<T>::value && std::is_enum_v<T>;

template<typename T> constexpr auto Get() {
    static_assert(is_described_v<T>, "Please use DESCRIBE() macro");
    constexpr auto res = GetDescription(Tag<T>{});
    return res;
}

template<typename Cls, auto...fields>
constexpr auto Describe(std::string_view clsname, std::string_view names) {
    Description<Cls, void, Field<fields>...> result = {};
    result.name = clsname;
    (static_cast<void>(result.template get<fields>().name = detail::next_name(names)), ...);
    return result;
}

template<typename Cls, auto...fields, typename ParCls, typename ParCls2, auto...parFields>
constexpr auto Describe(Description<ParCls, ParCls2, Field<parFields>...> parent,
                        std::string_view clsname,
                        std::string_view names)
{
    Description<Cls, ParCls, Field<parFields>..., Field<fields>...> result = {};
    result.name = clsname;
    (static_cast<void>(result.template get<parFields>().name = parent.template get<parFields>()), ...);
    (static_cast<void>(result.template get<fields>().name = detail::next_name(names)), ...);
    return result;
}

template<typename Cls> struct _ {
    template<auto...fs, typename...Args>
    static constexpr auto desc(Args...args) {
        return Describe<Cls, fs...>(args...);
    }
};

#define ALLOW_DESCRIBE_FOR(cls) \
friend constexpr auto GetDescription(::describe::Tag<cls>);

#define _D_DESCRIBE(cls, ...) ::describe::_<cls>::template desc<__VA_ARGS__>

#define DESCRIBE(cls, ...) \
inline constexpr auto GetDescription(::describe::Tag<cls>) { using _ [[maybe_unused]] = cls; \
return _D_DESCRIBE(cls, __VA_ARGS__)(#cls, #__VA_ARGS__);}

#define DESCRIBE_INHERIT(cls, parent, ...) \
inline constexpr auto GetDescription(::describe::Tag<cls>) { using _ = cls; \
return _D_DESCRIBE(cls, __VA_ARGS__)(::describe::Get<parent>(), #cls, #__VA_ARGS__);}

#define DESCRIBE_TEMPL_CLASS(...) \
inline constexpr auto GetDescription(::describe::Tag<__VA_ARGS__>) { \
using _ = __VA_ARGS__; constexpr std::string_view _clsName = #__VA_ARGS__;

#define DESCRIBE_TEMPL_FIELDS(...) \
return _D_DESCRIBE(_, __VA_ARGS__)(_clsName, #__VA_ARGS__); }

//! @achtung @warning THESE Should always be the same for all Translation Units
#define DESCRIBE_ATTRS(cls, ...) \
inline constexpr auto GetAttrs(::describe::Tag<cls>) { \
return ::describe::Attrs<__VA_ARGS__>{};}

//! @achtung @warning THESE Should always be the same for all Translation Units
#define DESCRIBE_FIELD_ATTRS(cls, field, ...) \
inline constexpr auto GetAttrs(::describe::Tag<cls>, ::describe::Field<&cls::field>) { \
return ::describe::Attrs<__VA_ARGS__>{}; }

// Utils
template<typename T>
constexpr auto field_names() {
    constexpr auto desc = describe::Get<T>();
    std::array<std::string_view, desc.fields_count> result;
    size_t idx = 0;
    desc.for_each_field([&](auto f){
        result[idx++] = f.name;
    });
    return result;
}

template<typename Enum>
constexpr auto min_max_value_of() {
    using under = std::underlying_type_t<Enum>;
    under n = 0, m = 0;
    Get<Enum>().for_each_field([&](auto f){
        auto curr = under(f.value);
        m = curr > m ? curr : m;
        n = curr < n ? curr : n;
    });
    return std::make_pair(n, m);
}

template<typename Enum>
constexpr auto make_lookup_table() {
    using under = std::underlying_type_t<Enum>;
    constexpr auto minman = min_max_value_of<Enum>();
    constexpr auto offs = minman.first - 0;
    constexpr auto diff = minman.second - minman.first;
    std::array<std::string_view, diff + 1> result = {};
    Get<Enum>().for_each_field([&](auto f){
        result[under(f.value) - offs] = f.name;
    });
    return result;
}

template<typename Enum> [[nodiscard]]
constexpr bool enum_to_name(Enum value, std::string_view& out) {
    static_assert(std::is_enum_v<Enum>);
    using under = std::underlying_type_t<Enum>;
    constexpr auto minman = min_max_value_of<Enum>();
    constexpr auto offs = minman.first - 0;
    constexpr auto diff = minman.second - minman.first;
    out = {};
    if constexpr (diff <= 256) {
        constexpr auto lookup = make_lookup_table<Enum>();
        if (under(value) >= minman.first && under(value) <= minman.second) {
            out = lookup[size_t(under(value) - offs)];
        }
    } else {
        Get<Enum>().for_each_field([&](auto f){
            if (!out.size() && f.value == value) {
                out = f.name;
            }
        });
    }
    return bool(out.data());
}

template<typename Enum> [[nodiscard]]
constexpr bool name_to_enum(std::string_view name, Enum& out) {
    static_assert(std::is_enum_v<Enum>);
    bool wasFound = false;
    Get<Enum>().for_each_field([&](auto f){
        if (!wasFound && f.name == name) {
            out = f.value; wasFound = true;
        }
    });
    return wasFound;
}

} //desc

#endif //DESCRIBE_HPP
