#pragma once
#include <vector>
#include <string>
#include <memory>

//Bare-bones JSON writer meant only for outputting a report for the unit tests
//Does *not* support unicode characters, or any of the null-ish values

class JsonNode;

enum class JsonType
{
    Boolean,
    Integer,
    String,
    Object,
    Array
};

class JsonNode
{
protected:
    JsonType nodeType;
    std::unique_ptr<JsonNode> value;
public:
    JsonNode(JsonType type) : nodeType{type}
    {}

    virtual std::string serialize() = 0;

    virtual ~JsonNode() = default;
};

class JsonBoolean : public JsonNode
{
    bool value = false;
public:
    JsonBoolean(bool _value) 
    : JsonNode{JsonType::Boolean}
    , value{_value}
    {}

    std::string serialize() override;
};

class JsonInteger : public JsonNode
{
    int64_t value = 0;
public:
    JsonInteger(int64_t _value)
    : JsonNode{JsonType::Integer}
    , value{_value}
    {}

    std::string serialize() override;
};

class JsonString: public JsonNode
{
    std::string value;
public:
    JsonString(std::string _value)
    : JsonNode{JsonType::String}
    , value{_value}
    {}

    std::string serialize() override;
};

class JsonArray: public JsonNode
{
    std::vector<std::unique_ptr<JsonNode>> values;
public:
    JsonArray()
    : JsonNode{JsonType::Array}
    {}

    std::string serialize() override;

    void AddElement(std::unique_ptr<JsonNode> nextElement);
};

struct JsonKeyValue
{
    std::string key;
    std::unique_ptr<JsonNode> value;
};

class JsonObject : public JsonNode
{
    std::vector<JsonKeyValue> attributes;

    bool keyAlreadyExists(const std::string& key);
public:
    JsonObject() : JsonNode(JsonType::Object)
    {}

    std::string serialize() override;

    void AddBool(std::string key, bool value);
    void AddString(std::string key, std::string value);
    void AddInteger(std::string key, int64_t value);
    void AddNode(std::string key, std::unique_ptr<JsonNode> node); //Array & Object
};

template<typename T, typename TCriteria=void>
struct json_type_map{};

template<typename T>
struct json_type_map<T, std::enable_if_t<std::is_integral<T>::value>>
{ 
    using type = JsonInteger;
};


template<>
struct json_type_map<bool>
{
    using type = JsonBoolean;
};

template<typename TString>
struct json_type_map<TString, std::enable_if_t<std::is_convertible<TString, std::string>::value>>
{
    using type = JsonString;
};


template<typename T>
struct json_type_map<
        std::unique_ptr<T>, 
        std::enable_if_t<std::is_base_of<JsonNode, T>::value>
    >
{
    using type = T;
};

namespace 
{
    using namespace std;
    void type_map_tests() //Sanity check type relation mapper for make_json_array helper
    {
        static_assert(is_same<json_type_map<int>::type, JsonInteger>::value, "int should map to 64bit int");
        static_assert(is_same<json_type_map<long>::type, JsonInteger>::value, "long should map to 64bit int");
        static_assert(is_same<json_type_map<short>::type, JsonInteger>::value, "short should map to 64bit int");

        static_assert(is_same<json_type_map<bool>::type, JsonBoolean>::value, "bool should map to bool");

        static_assert(is_same<json_type_map<string>::type, JsonString>::value, "string should map to string");
        static_assert(is_same<json_type_map<decltype("string literal")>::type, JsonString>::value, "string literal should map to string");

        static_assert(is_same<
            json_type_map<unique_ptr<JsonArray>>::type,
            JsonArray>::value,
            "Json Array should map to Json Array"
        );

        static_assert(is_same<
            json_type_map<unique_ptr<JsonObject>>::type,
            JsonObject>::value,
            "Json Object should map to Json Object"
        );
    }
}

template<typename T>
void CreateOrMoveArgument(JsonArray& jsonArray, T&& value)
{
    jsonArray.AddElement(
        std::make_unique<
            typename json_type_map<
                typename std::remove_reference<T>::type
            >::type
        >(std::forward<T>(value))
    );
}

template<typename T>
void CreateOrMoveArgument(JsonArray& jsonArray, unique_ptr<T> value)
{
    jsonArray.AddElement(move(value));
}

template<typename... TArgs>
std::unique_ptr<JsonArray> make_json_array(TArgs&&... args)
{
    std::unique_ptr<JsonArray> jsonArray = std::make_unique<JsonArray>();

    std::ignore = std::initializer_list<bool>{
       (CreateOrMoveArgument(*jsonArray, std::forward<TArgs>(args)), true)...
    };
    return jsonArray;
}


std::string StrJoin(const vector<std::string>& strings, const std::string& delimiter);