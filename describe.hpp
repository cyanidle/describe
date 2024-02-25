#ifndef DESCRIPTION_HPP
#define DESCRIPTION_HPP

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <utility>
#include <type_traits>

namespace description
{

template<auto field>
struct Field;

template<typename F, size_t...Is>
inline constexpr void do_for(F&& f, std::index_sequence<Is...>) {
    (f(std::integral_constant<size_t, Is>{}), ...);
}

template<size_t N, typename F>
inline constexpr void do_for(F&& f) {
    do_for(f, std::make_index_sequence<N>{});
}

namespace detail
{

template<size_t idx, typename T = void, typename...Ts>
struct pack_elem {
    using type = typename pack_elem<idx-1, Ts...>::type;
};
template<typename T, typename...Ts>
struct pack_elem<0, T, Ts...> {
    using type = T;
};

constexpr std::string_view get_next_stripped(std::string_view& src) {
    auto nextComma = src.find_first_of(',');
    auto result = src.substr(0, nextComma);
    src = src.substr(nextComma+1);
    if (auto pref = result.find_first_not_of(' '); pref != std::string_view::npos) {
        result = result.substr(pref);
    }
    if (auto pref = result.find_last_of(':'); pref != std::string_view::npos) {
        result = result.substr(pref + 1);
    }
    if (auto suff = result.find_last_not_of(' '); suff != std::string_view::npos) {
        result = result.substr(0, suff+1);
    }
    // todo: assert every field has name (and attrs?)
    return result;
}

template <class C, typename T>
T get_memptr_type(T C::*);
template <class C, typename T>
C& get_memptr_class(T C::*);
template <class C, typename T, typename...Args>
auto get_memptr_type(T (C::*v)(Args...)) -> decltype(v);
template <class C, typename T, typename...Args>
C& get_memptr_class(T (C::*)(Args...));

template<bool methods, typename...T>
constexpr size_t count() {
    return ((1 * (methods == T::is_method)) + ...);
}

}

template<auto field>
struct Field
{
    std::string_view name;
    static constexpr auto value = field;
    static constexpr auto is_method = std::is_member_function_pointer_v<decltype(field)>;
    using type = std::decay_t<decltype(detail::get_memptr_type(field))>;
    using cls = std::decay_t<decltype(detail::get_memptr_class(field))>;
};

template<typename Head, typename...Fields>
struct Description : protected Head, Fields...
{
    std::string_view cls_name;
    std::string_view ns_name;
    static constexpr auto fields_count = detail::count<false, Head, Fields...>();
    static constexpr auto methods_count = detail::count<true, Head, Fields...>();
    static constexpr auto all_count = fields_count + methods_count;
    template<size_t idx> using field_elem_t = typename detail::pack_elem<idx, Head, Fields...>::type;
    template<size_t idx> constexpr const auto& get() const {return static_cast<const field_elem_t<idx>&>(*this); }
    template<size_t idx> constexpr auto& get() {return static_cast<field_elem_t<idx>&>(*this); }
    using cls = typename Head::cls;
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

template<typename T> struct Tag {};
template<typename T, typename=void> struct has : std::false_type {};
template<typename T> struct has<T, std::void_t<decltype(GetDescription(Tag<T>{}))>> : std::true_type {};
template<typename T> constexpr auto has_v = has<T>::value;
template<typename T> constexpr auto Get() {return GetDescription(Tag<T>{});}

template<auto...fields>
constexpr auto Make(std::string_view clsname, std::string_view names) {
    constexpr auto AllCount = sizeof...(fields);
    using Result = Description<Field<fields>...>;
    static_assert(AllCount);
    static_assert((std::is_member_pointer_v<decltype(fields)> && ...),
                  "DESCRIBE() must be used with &Class::members only");
    Result result = {};
    result.cls_name = detail::get_next_stripped(clsname);
    auto diff = clsname.size() - result.cls_name.size();
    if (diff) {
        result.ns_name = clsname.substr(0, diff - 2); // (abs(::))cls_name
    }
    using cls = typename decltype(result)::cls;
    do_for<AllCount>([&](auto i) mutable {
        auto& field = result.template get<i()>();
        using currect_cls = typename std::decay_t<decltype(field)>::cls;
        static_assert(std::is_same_v<cls, currect_cls>,
                      "All fields must be From the same Type");
        field.name = detail::get_next_stripped(names);
    });
    return result;
}

#define _STRINGIFY2(...) #__VA_ARGS__
#define _STRINGIFY(...) _STRINGIFY2(__VA_ARGS__)
#define DESCRIBE(cls, ...) \
inline constexpr auto GetDescription(::description::Tag<cls>) { using _ = cls; \
    return ::description::Make<__VA_ARGS__>(_STRINGIFY(cls), _STRINGIFY(__VA_ARGS__)); \
}

} //desc

#endif //DESCRIPTION_HPP
