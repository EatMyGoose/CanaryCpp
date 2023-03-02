#pragma once

#include <string>
#include <vector>

#include "StringConverter.h"


namespace CTest
{
    //Like printf, but using %t to represent placeholders instead
    //i.e. tfmt("%t bottles of %t on the wall, take %t down, %t to go", 99, "beer" 1, 98)
    //The type provided in the argument list must either be a type that
    //  a) is convertible to a string OR
    //  b) has a free-standing to_string() function defined 
    //double up percentage signs to escape the % character
    template<typename ...TArgs>
    std::string cfmt(const std::string& format, TArgs&&... args)
    {
        std::vector<std::string> tokens = {
            (StrConverter::str_converter<TArgs>::get(args))...
        };

        size_t currentTokenIndex = 0;
        std::string output;
        for(size_t i = 0; i < format.size(); i++)
        {
            const char current = format.at(i); 
            if(current != '%')
            {
                output.push_back(current);
            }
            else
            {
                const size_t nextIdx = i + 1;
                bool validTokenAfterPercent = false;

                if(nextIdx < format.size())
                {
                    const char nextChar = format.at(nextIdx);
                    if(nextChar == '%')
                    {
                        i++;
                        validTokenAfterPercent = true;
                        output.push_back('%');        
                    }
                    else if(nextChar == 't')
                    {
                        i++;
                        validTokenAfterPercent = true; 
                        
                        if(currentTokenIndex < tokens.size())
                        {
                            output += tokens.at(currentTokenIndex);
                            currentTokenIndex++;
                        }
                        else
                        {
                            output += "[missing token]";
                        }
                    }
                }

                if(!validTokenAfterPercent)
                {
                    output += "[t or % expected after %]"; 
                }
            }
        }
        return output;
    }
}