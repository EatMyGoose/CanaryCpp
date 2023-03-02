#pragma once
#include <string>

struct UDF_With_Tostring
{
    int value;
    bool operator==(const UDF_With_Tostring& other) const {return value == other.value;}
    bool operator!=(const UDF_With_Tostring& other) const {return value != other.value;}
};

/*ODR*/ inline  std::string to_string(const UDF_With_Tostring&)
{
    return "user-defined-tostring";
}