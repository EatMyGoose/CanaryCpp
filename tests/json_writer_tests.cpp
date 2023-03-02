#include "..\CTest.h"
#include "..\jsonWriter.h"
#include <string>

std::string RemoveAllWhitespace(const std::string& input)
{
    std::string stripped;
    stripped.reserve(input.size());
    for(char c: input)
    {
        if(!isspace(c)) stripped.push_back(c);
    }
    return stripped;
}

TEST_METHOD(Whitespace_Stripper)
{
    test.assert(RemoveAllWhitespace("\none\ttwo three") == "onetwothree", "1) whitespace removal");
    test.assert(RemoveAllWhitespace("\n\t ") == "", "2) whitespace removal");
}


TEST_METHOD(Json_Writer_Tests)
{
    {
        auto obj = make_unique<JsonObject>();
        obj->AddString("string-key", "value1");
        obj->AddInteger("neg-int", -1);
        obj->AddInteger("pos-int", 1);
        obj->AddBool("false-bool", false);
        obj->AddBool("true-bool", true);

        test.assert(
            RemoveAllWhitespace(obj->serialize()) == 
            R"_({"string-key":"value1","neg-int":-1,"pos-int":1,"false-bool":false,"true-bool":true})_", 
            "Primitives serialized correctly"
        );
    }

    {
        auto obj = make_unique<JsonObject>();
        obj->AddString("key", "value");

        auto arr = make_json_array("one", 2, 3, true, false, move(obj));

        test.assert(
            RemoveAllWhitespace(arr->serialize()) == 
            R"_(["one",2,3,true,false,{"key":"value"}])_", 
            "Array serialized correctly"
        );
    }

    {
        auto objOuter = make_unique<JsonObject>();
        objOuter->AddInteger("int", 1);
        for(int i = 2; i <= 3; i++)
        {
            auto newOuter = make_unique<JsonObject>();
            newOuter->AddInteger("int", i);
            newOuter->AddNode("nested", move(objOuter));
            objOuter = move(newOuter);
        }   
         test.assert(
            RemoveAllWhitespace(objOuter->serialize()) == 
            R"_({"int":3,"nested":{"int":2,"nested":{"int":1}}})_", 
            "Nested objects"
        );
    }

    {
        auto nestedArray = make_json_array(
            1, make_json_array(2, 
                make_json_array(3, "four")
            )
        );
        test.assert(
            RemoveAllWhitespace(nestedArray->serialize()) == 
            R"_([1,[2,[3,"four"]]])_", 
            "Nested arrays"
        );
    }
}