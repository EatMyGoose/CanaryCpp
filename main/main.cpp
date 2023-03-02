#include "..\CTest.h"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    const auto testResults = CTest::Canary::Instance().RunAllTests();

    const string txtReport = CTest::FormatAsText(testResults, CTest::TextLogVerbosity::alwaysPrintAdditionalDetails);
    const string jsonReport = CTest::JsonifyTestResults(testResults);

    cout 
        << jsonReport
        << endl
        << "------------" << endl
        << txtReport
        << endl;

    
    ofstream jsonOfs(R"_(.\sample_output\JsonOutput.json)_");
    if(jsonOfs)
    {
        jsonOfs << jsonReport;
    }

    ofstream txtOfs(R"_(.\sample_output\TextReport_Verbose.txt)_");
    if(txtOfs)
    {
        txtOfs << txtReport;
    }

    return 0;
}