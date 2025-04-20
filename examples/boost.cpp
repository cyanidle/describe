// This is an example of simple integration with a library (attributes not used here)
// Just 4 functions enable boost::json basic conversion for all described structs and enums
#ifndef DESC_BOOST_CONV_HPP
#define DESC_BOOST_CONV_HPP

#include <boost/json/pilfer.hpp>
#include <describe/describe.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <string_view>
#include <string>
#include <vector>

template<typename T, describe::if_described_struct_t<T, int> = 1>
void tag_invoke(boost::json::value_from_tag, boost::json::value& j, T const& v) {
    auto& obj = j.emplace_object();
    describe::Get<T>::for_each([&](auto f){
        if constexpr (f.is_field) {
            obj[f.name] = boost::json::value_from(f.get(v));
        }
    });
}

template<typename T, describe::if_described_struct_t<T, int> = 1>
T tag_invoke(boost::json::value_to_tag<T>, const boost::json::value& v ) {
    T val;
    auto& obj = v.as_object();
    describe::Get<T>::for_each([&](auto f){
        if constexpr (f.is_field) {
            f.get(val) = boost::json::value_to<decltype(of(f))>(obj.at(f.name));
        }
    });
    return val;
}

template<typename T, describe::if_described_enum_t<T, int> = 1>
void tag_invoke(boost::json::value_from_tag, boost::json::value& j, T v) {
    auto& result = j.emplace_string();
    std::string_view name;
    if (!describe::enum_to_name(v, name)) {
        throw std::runtime_error(
            std::string{"Invalid value for enum ("}
            + std::string{describe::Get<T>::name} + ") => "
            + std::to_string(std::underlying_type_t<T>(v))
            );
    }
    result = name;
}

template<typename T, describe::if_described_enum_t<T, int> = 1>
T tag_invoke(boost::json::value_to_tag<T>, const boost::json::value& v ) {
    auto&& str = v.as_string();
    T val;
    if (!describe::name_to_enum(str, val)) {
        throw std::runtime_error(
            std::string{"Invalid value for enum ("}
            + std::string{describe::Get<T>::name} + ") => "
            + std::string(str)
            );
    }
    return val;
}

#endif // DESC_BOOST_CONV_HPP

#include <iostream>
#include <vector>
#include <list>
#include <boost/json.hpp>

using namespace boost;
using namespace std;

struct Person {
    string name;
    int age;
};

DESCRIBE("Person", Person) {
    // inside this scope: `using _ = Person`
    MEMBER("name", &_::name);
    MEMBER("age", &_::age);
}

struct House {
    string street;
    int number;
    std::vector<Person> residents;
};

DESCRIBE("House", House) {
    MEMBER("street", &_::street);
    MEMBER("number", &_::number);
    MEMBER("residents", &_::residents);
}

int main()
{
    std::vector<House> town;
    town.push_back(House{
        "Nice.Street", 1,
        {
            {"Steve", 23},
            {"Stevette", 24},
        }
    });
    town.push_back(House{
        "NotNice.Street", 32,
        {
            {"AntiSteve", 123},
            {"AntiStevette", 124},
        }
    });

    auto json = json::value_from(town);

    std::cout << "Whole town: \n\n";
    std::cout << json::serialize(json) << std::endl << std::endl;

    // now we use list (compatible with vector in Boost.Json)
    auto houses_list = json::value_to<std::list<House>>(json);

    std::cout << "1st house: \n\n";
    std::cout << json::serialize(json::value_from(houses_list.front())) << std::endl << std::endl;
}