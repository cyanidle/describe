#ifndef DESCRIBE_HPP
#define DESCRIBE_HPP
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <type_traits>
namespace describe
{

template<typename...Ts> struct Attrs {};
// this attr disables fields from inheriting attrs from their class
struct FieldsDoNotInherit {};
template<typename T> struct Tag {using type = T;};
template<auto field> struct Field;
template<typename T> constexpr auto Get();

template<typename F, size_t...Is>
inline constexpr void do_for(F&& f, std::index_sequence<Is...>) {
    (f(std::integral_constant<size_t, Is>{}), ...);
}
template<size_t N, typename F>
inline constexpr void do_for(F&& f) {
    do_for(f, std::make_index_sequence<N>{});
}
namespace det {

template<size_t idx, typename T = void, typename...Ts>
struct pack_elem {
    using type = typename pack_elem<idx-1, Ts...>::type;
};

template<typename T, typename...Ts>
struct pack_elem<0, T, Ts...> {
    using type = T;
};

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

auto get_attrs(...) -> Attrs<>;
template<typename...Ts> auto get_attrs(const Attrs<Ts...>&) -> Attrs<Ts...>;

template<typename F, typename = void>
struct has_GetAttrs : std::false_type {};

template<typename C>
struct has_GetAttrs<C, std::void_t<
                           decltype(GetAttrs(Tag<C>{}))
                           >>: std::true_type {};

template<auto field>
struct has_GetAttrs<Field<field>, std::void_t<
                                      decltype(GetAttrs(Tag<typename Field<field>::cls>{}, Field<field>{}))
                                      >>: std::true_type {}; // without Tag<Cls> as first arg ADL does not work!

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
} //det

template<typename T, typename = void>
struct get_attrs;

template<typename T>
using get_attrs_t = typename get_attrs<T>::type;

template<typename T, typename From>
using extract_attr_t = typename decltype(det::get_attr<T>(get_attrs_t<From>{}))::type;

template<typename T, typename Who>
constexpr bool has_attr_v = !std::is_void_v<extract_attr_t<T, Who>>;

template<auto field> struct Field {
    static_assert(
        std::is_member_pointer_v<decltype(field)> || std::is_enum_v<decltype(field)>,
        "Field can only be used with &_::members or enums");
    std::string_view name;
    static constexpr auto value = field;
    using type = typename decltype(det::get_memptr_type(field))::type;
    static constexpr auto is_method = std::is_member_function_pointer_v<type>;
    static constexpr auto is_enum = std::is_enum_v<type>;
    using cls = typename decltype(det::get_memptr_class(field))::type;
    template<typename T> static constexpr auto&& get(T&& obj) noexcept {
        return std::forward<T>(obj).*field;
    }
};

template<typename Cls, typename...Fields>
struct Description : protected Fields...
{
    using type = Cls;
    std::string_view name;
    std::string_view ns;
    static constexpr auto is_enum = std::is_enum_v<Cls>;
    static constexpr auto fields_count = det::count<false, Fields...>();
    static constexpr auto methods_count = det::count<true, Fields...>();
    static constexpr auto all_count = fields_count + methods_count;
    template<size_t idx> using field_elem_t = typename det::pack_elem<idx, Fields...>::type;
    template<size_t idx> constexpr const auto& get() const noexcept {
        return static_cast<const field_elem_t<idx>&>(*this);
    }
    template<size_t idx> constexpr auto& get() noexcept {
        return static_cast<field_elem_t<idx>&>(*this);
    }
    template<typename F> constexpr void for_each_field(F&& f) const {
        do_for<all_count>([&](auto i){
            if constexpr (!field_elem_t<i>::is_method) f(this->get<i()>());
        });
    }
    template<typename F> constexpr void for_each_method(F&& f) const {
        do_for<all_count>([&](auto i){
            if constexpr (field_elem_t<i>::is_method) f(this->get<i()>());
        });
    }
    template<typename F> constexpr void for_each(F&& f) const {
        do_for<all_count>([&](auto i){
            f(this->get<i()>());
        });
    }
};

template<typename T, typename>
struct get_attrs {
    using type = decltype(det::get_attrs(std::declval<const T&>()));
};

template<typename T>
struct get_attrs<T, std::enable_if_t<det::has_GetAttrs<T>::value>> {
    using type = decltype(GetAttrs(Tag<T>{}));
};

template<auto field>
struct get_attrs<Field<field>, std::enable_if_t<!det::has_GetAttrs<Field<field>>::value>> {
    using cls = typename Field<field>::cls;
    using type = std::conditional_t<has_attr_v<FieldsDoNotInherit, cls>, Attrs<>, get_attrs_t<cls>>;
};

template<auto field>
struct get_attrs<Field<field>, std::enable_if_t<det::has_GetAttrs<Field<field>>::value>> {
    // without Tag<Cls> as first arg ADL does not work!
    using type = decltype(GetAttrs(Tag<typename Field<field>::cls>{}, Field<field>{}));
};

template<typename Cls, auto...fs>
struct get_attrs<Description<Cls, Field<fs>...>> : get_attrs<Cls> {};

template<typename Cls>
struct get_attrs<const Cls> : get_attrs<Cls> {};

template<typename T, typename=void>
struct is_described : std::false_type {};

template<typename T>
struct is_described<T,
                    std::void_t<decltype(GetDescription(Tag<T>{}))
                                >>: std::true_type {};

template<typename T>
constexpr auto is_described_v = is_described<T>::value;

template<typename T> constexpr auto Get() {
    static_assert(is_described_v<T>, "Please use DESCRIBE() macro");
    return GetDescription(Tag<T>{});
}

template<typename Cls, auto...fields>
constexpr auto Describe(std::string_view clsname, std::string_view names) {
    Description<Cls, Field<fields>...> result = {};
    if (auto split = clsname.rfind("::"); split != std::string_view::npos) {
        result.ns = clsname.substr(0, split);
        result.name = clsname.substr(split + 2);
    } else {
        result.name = clsname;
    }
    do_for<sizeof...(fields)>([&](auto i) {
        auto& field = result.template get<i()>();
        static_assert(std::is_same_v<Cls, typename std::decay_t<decltype(field)>::cls>,
                      "All fields must be From the same Type");
        field.name = det::next_name(names);
    });
    return result;
}

namespace det {
template<typename Cls> struct descHelper {
    template<auto...fs> static constexpr auto Describe(std::string_view c, std::string_view ns) {
        return describe::Describe<Cls, fs...>(c, ns);
    }
};
} //det

#define ALLOW_DESCRIBE_FOR(cls) \
friend constexpr auto GetDescription(::describe::Tag<cls>);

#define DESCRIBE(cls, ...) \
inline constexpr auto GetDescription(::describe::Tag<cls>) { \
        using _ = cls; \
        return ::describe::det::descHelper<_>::template Describe<__VA_ARGS__>(#cls, #__VA_ARGS__); \
}

#define DESCRIBE_ATTRS(cls, ...) \
inline constexpr auto GetAttrs(::describe::Tag<cls>) { \
        return ::describe::Attrs<__VA_ARGS__>{}; \
}


#define DESCRIBE_FIELD_ATTRS(cls, field, ...) \
inline constexpr auto GetAttrs(::describe::Tag<cls>, ::describe::Field<&cls::field>) { \
        return ::describe::Attrs<__VA_ARGS__>{}; }

#define DESCRIBE_CLASS(...) \
inline constexpr auto GetDescription(::describe::Tag<__VA_ARGS__>) { \
        using _ = __VA_ARGS__; constexpr std::string_view _clsName = #__VA_ARGS__;

#define DESCRIBE_FIELDS(...) \
    return ::describe::det::descHelper<_>::template Describe<__VA_ARGS__>(_clsName, #__VA_ARGS__); }
} //desc

#endif //DESCRIBE_HPP
