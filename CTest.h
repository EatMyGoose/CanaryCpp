#pragma once
#include <functional>
#include <vector>
#include <string>
#include <type_traits>
#include "StringConverter.h"

namespace CTest
{
    using namespace std;

    enum class AssertType
    {
        plain_assert,
        assert_equals,
        assert_notequals,
        assert_throws,
        assert_nothrow
    };

    enum class TextLogVerbosity
    {
        printAdditionalDetailsOnFailingTests,
        alwaysPrintAdditionalDetails,
        neverPrintAdditionalDetails
    };

    struct AssertResult
    {
        AssertType assertType;
        bool passed;
        string description;
        string additionalDetails; //Additional info like "expected" & "actual" value
    };

    struct TestResults
    {
        string methodName;
        string groupName;
        vector<AssertResult> assertionResults;
        vector<string> logs;
        int64_t executionTimeMillis;

        using TPassedCases = size_t;
        using FailedCases = size_t;
        pair<TPassedCases, FailedCases> GetNumberOfPassedAndFailedCases() const;
    };

    class Tester
    {
    private:
        TestResults& boundResults;

        void AddAssertResult(AssertType enType, bool passed, const string& description, const string& details);
        void TestForThrow(const bool throwExpected, std::function<void(void)>& expr, const string& description);

    public:
        Tester(TestResults& boundResults);
        void log(const string& message);

        void assert(bool value, const string& description);

        void assert_throw(std::function<void(void)> expr, const string& description);
        void assert_nothrow(std::function<void(void)> expr, const string& description);

        template<typename T>
        void assert_eq(const T& actual, const T& expected, const string& description)
        {
            static_assert(
                StrConverter::str_converter<T>::scheme != StrConverter::conversion_scheme::none,
                "assert_eq requires a to_string() function or string cast operator to log down tested values"
            );

            using namespace std;
            string details = 
                "Actual: " + StrConverter::str_converter<T>::get(actual) + 
                " |Expected: " + StrConverter::str_converter<T>::get(expected);

            AddAssertResult(
                AssertType::assert_equals,
                actual == expected,
                description,
                details
            );
        }

        template<typename T>
        void assert_neq(const T& actual, const T& comparedValue, const string& description)
        {
             static_assert(
                StrConverter::str_converter<T>::scheme != StrConverter::conversion_scheme::none,
                "assert_neq requires a to_string() function or string cast operator to log down tested values"
            );

            const string details = 
                "Actual: " + StrConverter::str_converter<T>::get(actual) + 
                " |Compared Value: " + StrConverter::str_converter<T>::get(comparedValue);

            AddAssertResult(
                AssertType::assert_notequals,
                actual != comparedValue,
                description,
                details
            );
        }
    };

    using TTestMethod = function<void(Tester& tester)>; 

    class Canary
    {
        struct TestMethod
        {
            string name;
            string groupName;
            TTestMethod method;
        };

        vector<TestMethod> testMethodList;

        Canary() = default;
        
        static vector<TestResults> ExecuteTestMethods(vector<TestMethod>& methodList);
    public:
        static Canary& Instance();
        void AddTestMethod(const string& methodName, const string& groupName, TTestMethod testMethod);

        vector<TestResults> RunAllTests();
        vector<TestResults> RunTestGroup(const string& name);
    };

    class MethodRegistrar
    {
    public:
        MethodRegistrar(string methodName, string groupName, TTestMethod method);
    };

    string JsonifyTestResults(const vector<TestResults>& results);

    string FormatAsText(
        const vector<TestResults>& results, 
        enum TextLogVerbosity = TextLogVerbosity::printAdditionalDetailsOnFailingTests);
}

#define TEST_METHOD_NAME(METHOD_NAME) _test_method_##METHOD_NAME

#define TEST_GROUPED_METHOD(METHOD_NAME, LPSTR_GROUP_NAME)  \
    void TEST_METHOD_NAME(METHOD_NAME)(CTest::Tester&);     \
    static CTest::MethodRegistrar _test_registrar##METHOD_NAME(#METHOD_NAME, LPSTR_GROUP_NAME, TEST_METHOD_NAME(METHOD_NAME)); \
    void TEST_METHOD_NAME(METHOD_NAME)(CTest::Tester& test)

#define TEST_METHOD(METHOD_NAME) TEST_GROUPED_METHOD(METHOD_NAME, "")
    
    
    