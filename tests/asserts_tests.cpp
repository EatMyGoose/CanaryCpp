
#include "..\CTest.h"
#include ".\mock_udf.h"
#include <string>
#include <algorithm>

using namespace std;

TEST_METHOD(Assert_Throw_and_nothrow)
{
    auto throwingFunction = []{ throw runtime_error("exception thrown"); };

    test.assert_throw(throwingFunction, "Throws exception");

    auto nonThrowingFunction = []{ return; };

    test.assert_nothrow(nonThrowingFunction, "Does not throw exception");
}

struct no_str_conversion
{
    //no to_string(const&) defined or a cast operator, 
    //will not be compatible with assert_eq()/assert_neq()
    bool operator==(const no_str_conversion& other) {return true;};
};

TEST_METHOD(Assert_eq)
{
    test.assert_eq(1, 1, "1) Equality, primitive/library types");
    test.assert_eq(string("2"), string("2"), "2) Equality, primitive/library types");
    test.assert_eq("3", "3", "3) Equality, primitive/library types");
    test.assert_eq(true, true, "4) Equality, primitive/library types");
    test.assert_eq(UDF_With_Tostring{4}, UDF_With_Tostring{4}, "5) Equality, UDF Comparison");

    const string string1 = "1";
    test.assert_eq(string1, string1, "6) Equality, const specifier");

    #if 0 //Should not compile due to static_assert on the missing to_string and conversion operator 
    test.assert_eq(no_str_conversion{}, no_str_conversion{}, "should not compile");
    #endif
}

TEST_METHOD(Assert_neq)
{
    test.assert_neq(1, 2, "1) Inequality, primitive/library types");
    test.assert_neq(string("2"), string("3"), "2) Inequality, primitive/library types");
    test.assert_neq("3", "4", "3) Inequality, primitive/library types");
    test.assert_neq(true, false, "4) Inequality, primitive/library types");
    test.assert_neq(UDF_With_Tostring{4}, UDF_With_Tostring{5}, "5) Inequality, UDF Comparison");

    const string string1 = "1";
    const string string2 = "2";
    test.assert_neq(string1, string2, "6) Inequality, const specifier");
    #if 0 //Same as above (Assert_eq)
    test.assert_neq(no_str_conversion{}, no_str_conversion{}, "should not compile");
    #endif
}


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
    auto groupAResultList = CTest::Canary::Instance().RunTestGroup("group A");
    test.assert_eq(groupAResultList.size(), 2ULL, "1) Grouped results correctly filtered");

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
        methodNames == vector<string>{"GroupA_Method_1", "GroupA_Method_2"}, 
        "2) Grouped results correctly filtered"
    );
}