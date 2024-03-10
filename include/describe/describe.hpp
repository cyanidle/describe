#ifndef DESCRIPTION_HPP
#define DESCRIPTION_HPP

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <utility>
#include <type_traits>

namespace describe
{

template<typename T> struct Tag {using type = T;};
template<auto field> struct Field;

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
using sv = std::string_view;
// &optional::ba  , regex((foo|bar|baz), 123)::c,
constexpr void parse_one(sv& src, sv& meta, sv& name, bool isCls = false) {
    auto sepPos = src.find("::");
    auto comma = src.find_first_of(',', sepPos);
    auto nameBegin = sepPos != sv::npos ? sepPos + 2 : 0;
    auto nameEnd = isCls ? sv::npos : src.find_last_not_of(" \n\r\t", comma - 1);
    auto metaBegin = src.find_first_not_of("& \n\r\t");
    if (sepPos != sv::npos) {
        meta = src.substr(metaBegin, sepPos - metaBegin);
    }
    name = src.substr(nameBegin, nameEnd - nameBegin + 1);
    if(comma != sv::npos) {
        src = src.substr(comma + 1);
    }
}

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

}

template<auto field>
struct Field
{
    std::string_view name;
    std::string_view meta;
    static constexpr auto value = field;
    static constexpr auto is_method = std::is_member_function_pointer_v<decltype(field)>;
    using type = typename decltype(detail::get_memptr_type(field))::type;
    using cls = typename decltype(detail::get_memptr_class(field))::type;
    template<typename T> constexpr auto&& get(T&& obj) const noexcept {
        return std::forward<T>(obj).*value;
    }
};

template<typename Cls, typename...Fields>
struct Description : protected Fields...
{
    using type = Cls;
    std::string_view name;
    std::string_view meta;
    static constexpr auto fields_count = detail::count<false, Fields...>();
    static constexpr auto methods_count = detail::count<true, Fields...>();
    static constexpr auto all_count = fields_count + methods_count;
    template<size_t idx> using field_elem_t = typename detail::pack_elem<idx, Fields...>::type;
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

template<typename T, typename=void> struct is_described : std::false_type {};
template<typename T> struct is_described<T, std::void_t<decltype(GetDescription(Tag<T>{}))>> : std::true_type {};
template<typename T> constexpr auto is_described_v = is_described<T>::value;
template<typename T> constexpr auto Get() {return GetDescription(Tag<T>{});}

template<typename Cls, auto...fields>
constexpr auto Describe(std::string_view clsname, std::string_view names) {
    static_assert((std::is_member_pointer_v<decltype(fields)> && ...),
                  "DESCRIBE() must be used with &Class::members only");
    Description<Cls, Field<fields>...> result = {};
    detail::parse_one(clsname, result.meta, result.name, true);
    do_for<sizeof...(fields)>([&](auto i) {
        auto& field = result.template get<i()>();
        static_assert(std::is_same_v<Cls, typename std::decay_t<decltype(field)>::cls>,
                      "All fields must be From the same Type");
        detail::parse_one(names, field.meta, field.name);
    });
    return result;
}

#define ALLOW_DESCRIBE_FOR(cls) \
    friend constexpr auto GetDescription(::describe::Tag<cls>);
#define DESCRIBE(cls, ...) \
inline constexpr auto GetDescription(::describe::Tag<cls>) { using _ = cls; \
    return ::describe::Describe<_, ##__VA_ARGS__>(#cls, #__VA_ARGS__); \
}
#define TEMPL(cls, ...) cls<__VA_ARGS__>

} //desc

#endif //DESCRIPTION_HPP
