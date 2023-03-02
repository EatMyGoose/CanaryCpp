#include "..\CTest.h"
#include "..\formatter.h"
#include ".\mock_udf.h"

using namespace std;

TEST_METHOD(Formatter_Test)
{
    using namespace CTest;
    
    test.assert(cfmt("Escaped %%") == "Escaped %", "1) Percent correctly escaped");
    test.assert(cfmt("%%%% Escaped %%%%") == "%% Escaped %%", "2) Percent correctly escaped");
    test.assert(cfmt("%%") == "%", "3) Percent correctly escaped");

    test.assert(cfmt("%") == "[t or % expected after %]", "Missing token");

    test.assert(cfmt("%t dog", 1) == "1 dog", "int substitution");

    test.assert(cfmt("%t", true) == "true", "bool reflected as string literal(1)");
    test.assert(cfmt("%t", false) == "false", "bool reflected as string literal(2)");

    test.assert(cfmt("%t %tdog", 1, "cat") == "1 catdog", "1) int, string substitution");
    test.assert(cfmt("%t %tdog", 1, string("cat")) == "1 catdog", "2) int, string substitution");

    test.assert(cfmt("%t", UDF_With_Tostring{}) == "user-defined-tostring", "user-defined to_string for User-Defined Data Types");

    test.assert(cfmt("%t%% %%%t", 100, 100) == "100% %100", "No ambiguity between token and escaped percent");
}