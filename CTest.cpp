#include "CTest.h"
#include "jsonWriter.h"

#include <algorithm>
#include <chrono>

#include "formatter.h"

namespace CTest
{
#pragma region Tester
    Tester::Tester(TestResults& _boundResults)
        :boundResults(_boundResults)
    {}

    void Tester::AddAssertResult(
        AssertType enType, 
        bool passed, 
        const string& description,
        const string& details)
    {
        boundResults.assertionResults.emplace_back(
            AssertResult{
                enType,
                passed,
                description,
                details
            }
        );
    }

    void Tester::assert(bool expressionPassed, const string& description)
    {
        AddAssertResult(AssertType::plain_assert, expressionPassed, description, "");
    }    

    void Tester::TestForThrow(const bool throwExpected, std::function<void(void)>& expr, const string& description)
    {
        bool exceptionThrown = false;
        string stdExceptionMsg;
        try
        {
            expr();
        }
        catch(const exception& stdException)
        {
            exceptionThrown = true;
            stdExceptionMsg = "exception message: " + string(stdException.what());
        }
        catch(...)
        {
            exceptionThrown = true;
            stdExceptionMsg = "No message logged - exception does not inherit from std::exception";
        }

        const bool throwCriteriaMet = exceptionThrown == throwExpected;

        AddAssertResult(
            throwExpected? AssertType::assert_throws : AssertType::assert_nothrow,
            throwCriteriaMet, 
            description,
            stdExceptionMsg
        );
    }

    void Tester::assert_throw(std::function<void(void)> expr, const string& description)
    {
        TestForThrow(true, expr, description);
    }

    void Tester::assert_nothrow(std::function<void(void)> expr, const string& description)
    {
        TestForThrow(false, expr, description);
    }

    void Tester::log(const string& message)
    {
        boundResults.logs.emplace_back(message);
    }
#pragma endregion

#pragma region Canary
    Canary& Canary::Instance()
    {
        static Canary singleton;
        return singleton;
    }

    void Canary::AddTestMethod(const string& methodName, const string& groupName, TTestMethod testMethod)
    {
        testMethodList.emplace_back(
            TestMethod{
                methodName,
                groupName,
                testMethod
            }
        );
    }

    vector<TestResults> SortFailedTestFirst_ThenByGroup_ThenByAlphabeticalOrder(const vector<TestResults>& results)
    {
        using TNoFailures = bool;
        using TCachedFailureCount = pair<TNoFailures, const TestResults*>;
        vector<TCachedFailureCount> resultsToSort;
        //Cache whether each result set has failed test cases to avoid having the assertionResults vector
        //be scanned every comparison during the sort
        for(const TestResults& result: results)
        {
            size_t _discard = 0UL;
            size_t failedCases = 0UL;
            tie(_discard, failedCases) = result.GetNumberOfPassedAndFailedCases();
            
            const bool hasNoFailedTestCases = failedCases == 0;
            resultsToSort.emplace_back(
                make_pair(hasNoFailedTestCases,&result)
            );
        }

        sort(
            resultsToSort.begin(),
            resultsToSort.end(),
            [](const TCachedFailureCount& t1, TCachedFailureCount& t2)
            {
                return 
                    tie(t1.first, t1.second->groupName, t1.second->methodName) <
                    tie(t2.first, t2.second->groupName, t2.second->methodName);
            }
        );

        vector<TestResults> sortedWithFailuresFirstThenLexologically;
        for(TCachedFailureCount& sorted: resultsToSort)
        {
            sortedWithFailuresFirstThenLexologically.emplace_back(*sorted.second);
        }

        return sortedWithFailuresFirstThenLexologically;
    }

    vector<TestResults> Canary::ExecuteTestMethods(vector<TestMethod>& methodList)
    {
        vector<TestResults> results;

        transform(
            methodList.begin(), methodList.end(),
            back_inserter(results),
            [](TestMethod& testMethod)
            {
                TestResults testResultSet;
                Tester tester(testResultSet);

                const auto startTime = chrono::steady_clock::now();
                testMethod.method(tester);
                const auto endTime = chrono::steady_clock::now();

                const int64_t elapsedMillis = 
                    chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

                testResultSet.groupName = testMethod.groupName;
                testResultSet.methodName = testMethod.name;
                testResultSet.executionTimeMillis = elapsedMillis;
                return testResultSet;
            }
        );

        return SortFailedTestFirst_ThenByGroup_ThenByAlphabeticalOrder(results);
    }

    vector<TestResults> Canary::RunAllTests()
    {
       return ExecuteTestMethods(testMethodList);
    }

    vector<TestResults> Canary::RunTestGroup(const string& name)
    {
        vector<TestMethod> filteredMethods;
        copy_if(
            testMethodList.begin(), testMethodList.end(),
            back_inserter(filteredMethods),
            [&name](const TestMethod& results)
            {
                return results.groupName == name;
            }
        );

        return ExecuteTestMethods(filteredMethods);
    }

#pragma endregion

#pragma region TestResults
    pair<TestResults::TPassedCases, TestResults::FailedCases> 
        TestResults::GetNumberOfPassedAndFailedCases() const
    {
        size_t nPassingTests = 0;
        size_t nFailingTests = 0;

        for(const AssertResult& assertResult: assertionResults)
        {
            if(assertResult.passed) nPassingTests++;
            else nFailingTests++;
        }

        return make_pair(nPassingTests, nFailingTests);
    }
#pragma endregion

#pragma region MethodRegistrar
    MethodRegistrar::MethodRegistrar(string methodName, string groupName, TTestMethod method)
    {
        Canary::Instance().AddTestMethod(methodName, groupName, method);
    }
#pragma endregion

#pragma region FreeStandingFunctions
    struct OverallTestResults
    {
        size_t nTotalPassedTests = 0;
        size_t nTotalFailedTests = 0;
        int64_t totalTestTimeMillis = 0;
        size_t nTotalTests = 0;
    };

    OverallTestResults GetOverallTestResults(const vector<TestResults>& results)
    {
        OverallTestResults overallResults{};

        for(const TestResults& result: results)
        {
            size_t nPassed = 0;
            size_t nFailed = 0;
            tie(nPassed, nFailed) = result.GetNumberOfPassedAndFailedCases();
            overallResults.nTotalPassedTests += nPassed;
            overallResults.nTotalFailedTests += nFailed;
            overallResults.totalTestTimeMillis += result.executionTimeMillis;
        }
        overallResults.nTotalTests = 
            overallResults.nTotalPassedTests +
            overallResults.nTotalFailedTests;
        
        return overallResults;
    }

    std::string GetAssertTypeName(const AssertType enType)
    {
        switch(enType)
        {
            case AssertType::plain_assert:      return "assert";
            case AssertType::assert_equals:     return "eq";
            case AssertType::assert_notequals:  return "neq";
            case AssertType::assert_throws:     return "throw";
            case AssertType::assert_nothrow:    return "nothrow";
            default: return "[unknown]";
        }
    }

    std::string PadWithSpaces(const string& s, size_t length)
    {
        if(s.size() >= length) return s;

        string padded(length, ' ');

        for(size_t i = 0; i < s.size(); i++)
        {
            padded.at(i) = s.at(i);
        }

        return padded;
    }

    string JsonifyTestResults(const vector<TestResults>& results)
    {
        const OverallTestResults overallResults = GetOverallTestResults(results);
        const bool allTestsPassed = overallResults.nTotalFailedTests == 0;
        auto reportBody = make_unique<JsonObject>();
        
        reportBody->AddBool("all-tests-passed", allTestsPassed);
        reportBody->AddInteger("passing-tests", overallResults.nTotalPassedTests);
        reportBody->AddInteger("failing-tests", overallResults.nTotalFailedTests);
        reportBody->AddInteger("total-test-time-millis", overallResults.totalTestTimeMillis);

        auto testResults = make_unique<JsonArray>();
        for(const TestResults& result: results)
        {
            size_t nPassed = 0;
            size_t nFailed = 0;
            tie(nPassed, nFailed) = result.GetNumberOfPassedAndFailedCases();

            auto currentResult = make_unique<JsonObject>();
            currentResult->AddString("name", result.methodName);
            currentResult->AddString("group", result.groupName);
            currentResult->AddBool("all-passed", nFailed == 0);
            currentResult->AddInteger("passing-tests", nPassed);
            currentResult->AddInteger("failing-tests", nFailed);
            currentResult->AddInteger("test-time-millis", result.executionTimeMillis);

            {
                auto assertList = make_unique<JsonArray>();
                for(const AssertResult& assertResult: result.assertionResults)
                {
                    auto assertNode = make_unique<JsonObject>();
                    assertNode->AddString("type", GetAssertTypeName(assertResult.assertType));
                    assertNode->AddBool("passed", assertResult.passed);
                    assertNode->AddString("description", assertResult.description);
                    assertNode->AddString("details", assertResult.additionalDetails);

                    assertList->AddElement(move(assertNode));
                }
                currentResult->AddNode("assertions", move(assertList));
            }
            {
                auto logList = make_unique<JsonArray>();
                for(const string& log: result.logs)
                {
                    logList->AddElement(make_unique<JsonString>(log));
                }
                currentResult->AddNode("logs", move(logList));
            }

            testResults->AddElement(move(currentResult));
        }

        reportBody->AddNode("test-results", move(testResults));

        return reportBody->serialize();
    }

    bool ShouldPrintAdditionalDetails(
        bool testCasePassed, 
        enum CTest::TextLogVerbosity verbosity,
        const string& additionalDetails)
    {
        const bool detailsNotEmpty = !additionalDetails.empty();
        if(verbosity == CTest::TextLogVerbosity::alwaysPrintAdditionalDetails) 
        {
            return detailsNotEmpty;
        }
        else if(verbosity == CTest::TextLogVerbosity::neverPrintAdditionalDetails) 
        {
            return false;
        }
        else if(verbosity == CTest::TextLogVerbosity::printAdditionalDetailsOnFailingTests) 
        {
            return testCasePassed == false && detailsNotEmpty;
        }
        else
        {
            return false;
        }
    }

    string FormatAsText(const vector<TestResults>& results, enum TextLogVerbosity verbosity)
    {
        const OverallTestResults overallResults = GetOverallTestResults(results);
        const bool allCasesPassed = overallResults.nTotalFailedTests == 0;

        const string overallResultCaption = 
            allCasesPassed?
                "All tests passed" :
                "Failing tests detected";

        string header = cfmt("%t|%t/%t tests passed|Time taken:%tms",
                overallResultCaption,
                overallResults.nTotalPassedTests,
                overallResults.nTotalTests,
                overallResults.totalTestTimeMillis
            );
        
        vector<string> report;
        report.emplace_back(move(header));
        report.emplace_back("Test results:");

        for(const TestResults& testResult: results)
        {
            size_t nPassing = 0;
            size_t nFailing = 0;
            tie(nPassing, nFailing) = testResult.GetNumberOfPassedAndFailedCases();

            const string groupDisplay = 
                !testResult.groupName.empty()? cfmt(", group:%t", testResult.groupName):
                "";

            string testMethodOverview = cfmt("   Test Method:%t, passed %t/%t, all-passed?:%t, running time:%tms%t", 
                testResult.methodName,
                nPassing, nPassing + nFailing,
                (nFailing == 0)? "True": "False",
                testResult.executionTimeMillis,
                groupDisplay
            );

            report.emplace_back(move(testMethodOverview));
            for(const AssertResult& assertResult: testResult.assertionResults)
            {
                string line = cfmt("      %t - %t, Description [ %t ]",
                    assertResult.passed? "Passed" : "Failed",
                    PadWithSpaces(GetAssertTypeName(assertResult.assertType), 6),
                    assertResult.description
                );
                report.emplace_back(move(line));

                if(ShouldPrintAdditionalDetails(
                    assertResult.passed, 
                    verbosity, 
                    assertResult.additionalDetails) == false) continue;

                
                //Print out the logs (if any)
                string additionalDetails = cfmt("         Details: %t", 
                    assertResult.additionalDetails
                );
                report.emplace_back(move(additionalDetails));
            }
        }

        //Indicate overall pass/failure at the bottom to allow it to be 
        //more easily read at the end of console output
        report.emplace_back(cfmt("\n[---%t---]", overallResultCaption));
        return StrJoin(report, "\n");
    }
#pragma endregion
}
