#include "reporter.h"
#include <iostream>
#include <algorithm>

void Reporter::reportCompilationError(const CompileResult& res) {
    std::cout << "Compilation Failed\n\n";
    std::cout << "AI Explanation:\n";
    
    // Pattern Matching Logic from your Report
    if (res.errorOutput.find("cout") != std::string::npos && 
        res.errorOutput.find("not declared") != std::string::npos) {
        std::cout << "The compiler reports that 'cout' is not declared.\n"
                  << "This usually happens when the std:: namespace is missing.\n"
                  << "Use std::cout or add 'using namespace std;'.\n";
    }
    else if (res.errorOutput.find("No such file") != std::string::npos) {
        std::cout << "The specified source file could not be found.\n"
                  << "Please verify the file name and path.\n";
    }
    else {
        // Fallback Default from Spec
        std::cout << "The compiler encountered an error.\n"
                  << "Please review the syntax and declarations in your code.\n";
    }
}

void Reporter::reportExecution(const ExecutionResult& res) {
    std::cout << "Execution Output:\n";
    std::cout << res.output << "\n"; // Ensure newline at end if missing

    std::cout << "Execution Status: " << res.rawStatus << "\n";
    
    if (!res.interpretation.empty()) {
        std::cout << "Sandbox Interpretation: " << res.interpretation << "\n";
    }
}

// JSON logic remains for Web Interface compatibility
void Reporter::reportJson(const CompileResult& cRes, const ExecutionResult& eRes) {
    std::cout << "{";
    std::cout << "\"compiled\": " << (cRes.success ? "true" : "false") << ",";
    
    std::string safeCompileOut = cRes.errorOutput;
    std::replace(safeCompileOut.begin(), safeCompileOut.end(), '"', '\'');
    std::replace(safeCompileOut.begin(), safeCompileOut.end(), '\n', ' ');
    std::cout << "\"compileOutput\": \"" << safeCompileOut << "\",";

    if (cRes.success) {
        std::string safeExecOut = eRes.output;
        std::replace(safeExecOut.begin(), safeExecOut.end(), '"', '\'');
        std::replace(safeExecOut.begin(), safeExecOut.end(), '\n', ' ');

        std::cout << "\"executed\": true,";
        std::cout << "\"output\": \"" << safeExecOut << "\",";
        std::cout << "\"status\": \"" << eRes.rawStatus << "\",";
        std::cout << "\"interpretation\": \"" << eRes.interpretation << "\"";
    } else {
        std::cout << "\"executed\": false";
    }
    std::cout << "}\n";
}