#include <assert.h>
#include "jsonWriter.h"
#include <map>
#include <algorithm>

using namespace std; 

string StrJoin(const vector<string>& strings, const string& delimiter)
{
    if(strings.empty()) return "";

    string buffer;

    for(size_t i = 0; i < strings.size() - 1; i++)
    {
        buffer += strings[i];
        buffer += delimiter;
    }

    buffer += strings.back();

    return buffer;
}


string EscapeJsonString(const string& str)
{
    const static map<char, string> escapeChars{
        {'"', R"_(\")_"},
        {'\\',R"_(\\)_"},
        {'/',R"_(\/)_"},
        {'\b',R"_(\b)_"},
        {'\f',R"_(\f)_"},
        {'\n',R"_(\n)_"},
        {'\r',R"_(\r)_"},
        {'\t',R"_(\t)_"}
        //\u sequence completely ignored.
    };

    string escaped;
    for(const char c: str)
    {
        if(escapeChars.count(c) == 0)
        {
            //non-escaped character
            escaped.push_back(c);
        }
        else
        {
            escaped += escapeChars.at(c);
        }
    }
    return escaped;
}

#pragma region Boolean
 std::string JsonBoolean::serialize() 
 {
    return value? "true": "false";
 }
#pragma endregion

#pragma region Integer
 std::string JsonInteger::serialize() 
 {
    return to_string(value);
 }
#pragma endregion

#pragma region String
std::string JsonString::serialize()
{
    return "\"" + EscapeJsonString(value) + "\"";
}
#pragma endregion

#pragma region JsonArray
std::string JsonArray::serialize()
{
    vector<string> elements;
    for(unique_ptr<JsonNode>& pNode: values)
    {
        assert(pNode != nullptr);
        elements.emplace_back(pNode->serialize());
    }

    return "[" + StrJoin(elements, ",") + "]";
}

void JsonArray::AddElement(std::unique_ptr<JsonNode> nextElement)
{
    assert(nextElement != nullptr);
    values.emplace_back(move(nextElement));
}
#pragma endregion

#pragma region Object

string JsonObject::serialize() 
{
    string buffer;
    buffer += "{\n";

    vector<string> elements;
    for(JsonKeyValue& keyValue: attributes)
    {        
        assert(keyValue.value != nullptr);
        string line = '"' + keyValue.key + "\":" + keyValue.value->serialize();
        elements.emplace_back(move(line));
    }

    buffer += StrJoin(elements, ",\n");

    buffer += "\n}";
    return buffer;
}

bool JsonObject::keyAlreadyExists(const std::string& key)
{
    //Assuming JSON objects are fairly small,
    //Allocation overhead of a set/unordered_set seems a bit overkill
    const bool keyExists = std::find_if(
        attributes.begin(), attributes.end(),
        [&key](const JsonKeyValue& keyValue)
        {
            return keyValue.key == key;
        }
    ) != attributes.end();

    return keyExists;
}

void JsonObject::AddBool(std::string key, bool boolValue)
{
    if(keyAlreadyExists(key)) throw invalid_argument("key already exists");

    attributes.emplace_back(JsonKeyValue{
        key,
        make_unique<JsonBoolean>(boolValue)
    });
}

void JsonObject::AddString(std::string key, std::string strValue)
{
    if(keyAlreadyExists(key)) throw invalid_argument("key already exists");

    attributes.emplace_back(JsonKeyValue{
        key,
        make_unique<JsonString>(strValue)
    });
}

void JsonObject::AddInteger(std::string key, int64_t intValue)
{
    if(keyAlreadyExists(key)) throw invalid_argument("key already exists");

    attributes.emplace_back(JsonKeyValue{
        key,
        make_unique<JsonInteger>(intValue)
    });
}

void JsonObject::AddNode(std::string key, std::unique_ptr<JsonNode> node)
{
    if(keyAlreadyExists(key)) throw invalid_argument("key already exists");

    attributes.emplace_back(JsonKeyValue{
        key,
        move(node)
    });
}

#pragma endregion 