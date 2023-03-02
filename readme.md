# CanaryCpp

## About
A header only C++14 library for basic unit-testing. Targeted towards  environments where more established solutions may not be able to work (i.e. being unable to setup Visual Studio's test environment due to DLL dependency errors). 

## Running Tests
All tests are run by calling `CTest::Canary::Instance().RunAllTests()`. 
Tests can also be run by groups using `CTest::Canary::Instance().RunTestGroup(string groupName)`. 

Results can then be formatted into either plain-text or json via `CTest::JsonifyTestResults` or `CTest::FormatAsText`.

i.e.

```
int main()
{
    const auto testResults = 
        CTest::Canary::Instance().RunAllTests();

    const auto testGroupResults = 
        CTest::Canary::Instance().RunTestGroup("Group A");

    cout 
        << CTest::JsonifyTestResults(testResults) 
        << endl
        << CTest::FormatAsText(testGroupResults) 
        << endl;

    return 0;
}
```

Tests cases in reports are ordered by failing methods first, then by group, followed by test description.

## Writing Tests

Within any .cpp file included in the project build, include the `"CTest.h"` header. Write ungrouped test via the `TEST_METHOD(<method-name>)` macro. Within the test method body, use any of the 5 asserts types to create a unit-test condition. Multiple asserts can be used within the same `TEST_METHOD` macro.

```
TEST_METHOD(ExampleTest)
{
    test.assert(true, "1) Plain old assert");

    //assert_eq() and assert_neq() automatically 
    //log the expected and actual values in the report
    test.assert_eq(1, 1, "2) Equality assert");
    test.assert_neq(string("1"), string("2"), "3) Inequality assert");

    //Test for an expected exception
    auto throwingFunction = []{ throw runtime_error("exception thrown"); };
    test.assert_throw(
        throwingFunction, 
        "4) Throws exception");

    //Assert that an exception should *not* occur
    auto nonThrowingFunction = []{ return; };
    test.assert_nothrow(
        nonThrowingFunction, 
        "5) Does not throw exception");
}
```

Alternatively, mark a test as part of the group so it may be run separately via `RunTestGroup()`. Tests created via `TEST_METHOD()` are assigned an empty group name `""`.

```
TEST_GROUPED_METHOD(GroupA_Method_1, "group A")
{
    test.assert(true, "1) group A");
}

TEST_GROUPED_METHOD(GroupA_Method_2, "group A")
{
    test.assert(true, "2) group A");
}

TEST_GROUPED_METHOD(GroupMethodFilteredCorrectly, "group B")
{
    auto groupAResultList = 
        CTest::Canary::Instance().RunTestGroup("group A");

    vector<string> methodNames;
    std::transform(
        groupAResultList.begin(), groupAResultList.end(),
        back_inserter(methodNames),
        [](const CTest::TestResults& results)
        {
            return results.methodName;
        }
    );

    test.assert(
        methodNames == vector<string>{
            "GroupA_Method_1", "GroupA_Method_2"
        }, 
        "Grouped results correctly filtered"
    );
}
```


All test methods are registered at runtime and executed in essentially random order in the same address space as the callee. **Test cases which cause process termination cannot be handled**.

## Assert Types

```
test.assert(bool expression, string description)
```

**expression:** `true` if the test condition is satisfied. 

**description:** textual description of the assert that is displayed in the reports

```
template<typename T>
test.assert_eq(T& actual, T& expected, string description)

template<typename T>
test.assert_neq(T& actual, T& comparedValue, string description)
```
**actual:** Value of the test-expression

**expected:** The value that `actual` should be equal to to pass the test.

**comparedValue:** The value that `actual` should **not** be equal to to pass the test.

Note: `actual`, `expected` & `comparedValue` are recorded and reflected in the report (Note: by default the text report only shows values in failing cases).

**User-Defined Types**: `T` may be any compatible user defined type that fulfills the following:
1. Implements `==` (assert_eq) or `!=` (assert_neq)
2. Has either a `std::string to_string(const T&)` function defined **OR** a casting operator to `std::string`

```
test.assert_throw(
    function<void(void)> throw_expr, string description);

test.assert_nothrow(
    function<void(void)> nothrow_expr, string description);
```

**throw_expr:** Any function/functor that is expected to throw an exception. Use a enclosing lambda expression to curry in arguments if required.

**nothrow_expr:** Any function/functor that is not expected to throw an exception.

Note: For any exception derived from `std::exception`, the error message from `expection::what()` is automatically recorded. If this behaviour is required for other types of exceptions (i.e. MFC's `CException`), extend the try-catch blocks within `Tester::TestForThrow()`.

## Project Setup
Include the following files in your project:
```
formatter.h
jsonWriter.cpp
jsonWriter.h
StringConverter.h
CTest.cpp
CTest.h
```
Then include "CTest.h" in any .cpp file which requires access to the unit-testing functions.

Builds on GCC (c++14, no special flags required). Should have no issue with VS2015 & onwards (untested).

## Miscellaneous
A lightweight string-formatting function `CTest::cfmt()` is also available in the framework. Works like a regular `printf()` but with all tokens replaced with `%t` instead. Use `%%` to escape the percent sign. Outputs a std::string and accepts User-Defined-Types which fulfill the string conversion requirements for `test.assert_eq` & `test.assert_neq`.
```
string cfmt(string format, ...);

string result = 
    CTest::cfmt(
        "Escaped %%, token-1: %t, token-2: %t, token-3: %t",
        1,
        "two",
        true //booleans are converted to "true"/"false" strings
    );

//result = "Escaped %, token-1: 1, token-2: two, token-3: true"
```






