#pragma once
#include <type_traits>
#include <string>

//Helper template function to determine how to convert a type into a std::string
//(i.e. calling to_string() if defined or using the conversion operator if available)
namespace StrConverter
{
    //Indicator enum to enable sanity checks for the type-based dispatch
    enum class conversion_scheme 
    {
        none,
        convertible,
        to_string,
        bool_true_false
    };

    template<typename T, typename = void>
    struct str_converter
    {
        static constexpr conversion_scheme scheme = conversion_scheme::none;
    };
 
    template<typename T>
    struct str_converter<
                T, 
                std::enable_if_t<std::is_convertible<T, std::string>::value>
            >
    {
        static constexpr conversion_scheme scheme = conversion_scheme::convertible;
        static std::string get(const T& value) { return static_cast<std::string>(value); }
    };

    //avoid defining namespace explicity (i.e std::tostring) 
    //to allow to_string methods outside of std:: namespace to be found as well
    using namespace std; 
    template<typename T>
    struct str_converter<
                T,
                std::enable_if_t<
                    std::is_same<
                        decltype(to_string(std::declval<T>())), 
                        std::string>::value
                    >
            >
    {
        static constexpr conversion_scheme scheme = conversion_scheme::to_string;
        static string get(const T& value) {return to_string(value);}   
    };

    template<>
    struct str_converter<bool, void>
    {
        static constexpr conversion_scheme scheme = conversion_scheme::bool_true_false;
        static std::string get(const bool value) 
        { 
            return value? "true" : "false"; 
        }
    };
}

namespace
{
    struct custom_with_to_string{};
    std::string to_string(custom_with_to_string) {return "";}

    void str_converter_tests()
    {
        using namespace StrConverter;

        struct custom_no_conversion{};

        static_assert(
            str_converter<custom_no_conversion>::scheme == conversion_scheme::none, 
            "no conversion available for UDF");

        static_assert(
            str_converter<custom_with_to_string>::scheme == conversion_scheme::to_string, 
            "conversion available for UDF");
        
        static_assert(
            str_converter<string>::scheme == conversion_scheme::convertible, 
            "convertible to string");

        static_assert(
            str_converter<const char*>::scheme == conversion_scheme::convertible, 
            "convertible to string");
        
        static_assert(
            str_converter<int>::scheme == conversion_scheme::to_string, 
            "convertible to string");

        static_assert(
            str_converter<bool>::scheme == conversion_scheme::bool_true_false, 
            "bool serializes to true & false string literals");
    }
}

