#ifndef DESC_BOOST_CONV_HPP
#define DESC_BOOST_CONV_HPP

#include <describe/describe.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <string_view>
#include <string>

template<typename T, std::enable_if_t<describe::is_described_struct_v<T>, int> = 1>
void tag_invoke(boost::json::value_from_tag, boost::json::value& j, T const& v) {
    auto& obj = j.emplace_object();
    describe::Get<T>().for_each_field([&](auto f){
        obj[f.name] = boost::json::value_from(f.get(v));
    });
}

template<typename T, std::enable_if_t<describe::is_described_struct_v<T>, int> = 1>
T tag_invoke(boost::json::value_to_tag<T>, const boost::json::value& v ) {
    T val;
    auto& obj = v.as_object();
    describe::Get<T>().for_each_field([&](auto f){
        f.get(val) = boost::json::value_to<typename decltype(f)::type>(obj.at(f.name));
    });
    return val;
}

template<typename T, std::enable_if_t<describe::is_described_enum_v<T>, int> = 1>
void tag_invoke(boost::json::value_from_tag, boost::json::value& j, T v) {
    auto& s = j.emplace_string();
    std::string_view name;
    if (!describe::enum_to_name(v, name)) {
        throw std::runtime_error(
            std::string{"Invalid value for enum ("}
            + std::string{describe::Get<T>().name} + ") => "
            + std::to_string(std::underlying_type_t<T>(v))
            );
    }
    s = name;
}

template<typename T, std::enable_if_t<describe::is_described_enum_v<T>, int> = 1>
T tag_invoke(boost::json::value_to_tag<T>, const boost::json::value& v ) {
    auto&& str = v.as_string();
    T val;
    if (!describe::name_to_enum(str, val)) [[unlikely]] {
        throw std::runtime_error(
            std::string{"Invalid value for enum ("}
            + std::string{describe::Get<T>().name} + ") => "
            + std::string(str)
            );
    }
    return val;
}

#endif // DESC_BOOST_CONV_HPP