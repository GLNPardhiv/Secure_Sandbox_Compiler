#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>

#include "compiler.h"
#include "sandbox.h"
#include "reporter.h"
#include "security.h"

// --- Helper Function to Call Python AI ---
std::string callAIAnalyzer(const std::string& sourceFile) {
    std::string command = "python3 risk_analyzer.py " + sourceFile;
    std::array<char, 128> buffer;
    std::string result;
    
    // Open pipe to python script
    FILE* pipe = popen(command.c_str(), "r");
    
    if (!pipe) {
        return "{}"; // Failed to open pipe
    }

    // Read stdout from python script
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    // Close pipe
    pclose(pipe);
    
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./sandboxcc <source_file> [--run] [--json]\n";
        return 1;
    }

    std::string sourceFile = argv[1];
    bool runRequested = false;
    bool jsonMode = false;

    for(int i=2; i<argc; i++) {
        if (strcmp(argv[i], "--run") == 0) runRequested = true;
        if (strcmp(argv[i], "--json") == 0) jsonMode = true;
    }

    Compiler compiler;
    Sandbox sandbox;
    Reporter reporter;

    // --- PHASE 4: AI Risk Analysis (Week 9) ---
    if (!jsonMode) std::cout << ">>> 🧠 Invoking AI Risk Analyzer...\n";
    
    std::string aiJson = callAIAnalyzer(sourceFile);
    
    // Simple JSON parsing (manual string find for simplicity in C++)
    bool isSafe = true;
    if (aiJson.find("\"is_safe\": false") != std::string::npos) {
        isSafe = false;
    }

    // Extract Analysis Message
    std::string analysisMsg = "Analysis failed";
    size_t start = aiJson.find("\"analysis\": \"");
    if (start != std::string::npos) {
        start += 13; // Length of label
        size_t end = aiJson.find("\"", start);
        analysisMsg = aiJson.substr(start, end - start);
    }

    // --- DECISION GATE ---
    if (!isSafe) {
        if (jsonMode) {
            std::cout << "{"
                      << "\"compiled\": false,"
                      << "\"executed\": false,"
                      << "\"status\": \"Blocked by AI Security\","
                      << "\"interpretation\": \"" << analysisMsg << "\""
                      << "}\n";
        } else {
            std::cout << "\n[!] 🛑 SECURITY ALERT: Code blocked by AI Risk Analyzer.\n";
            std::cout << "[!] Reason: " << analysisMsg << "\n";
            std::cout << "[!] The compiler will NOT proceed.\n";
        }
        return 1; // STOP HERE
    }

    if (!jsonMode) std::cout << ">>> ✅ AI Analysis Passed. Proceeding to compilation.\n";

    // --- EXISTING FLOW (Weeks 1-8) ---

    // Phase 1: Compile (with Seccomp injection)
    CompileResult cRes = compiler.compile(sourceFile);
    
    ExecutionResult eRes;
    eRes.exitCode = 0;
    eRes.signal = 0;
    eRes.timeout = false;

    if (cRes.success) {
        if (!jsonMode) {
            std::cout << "Compilation: SUCCESS (Seccomp & Isolation Ready)\n";
        }
        
        if (runRequested || jsonMode) {
            if (!jsonMode) {
                std::cout << ">>> Setting up File System Jail...\n";
            }
            
            std::string jailedBinary = SecurityModule::setupJail(cRes.binaryPath);
            eRes = sandbox.execute(jailedBinary);
            SecurityModule::cleanupJail(jailedBinary);
            remove(cRes.binaryPath.c_str());

            if (!jsonMode) {
                reporter.reportExecution(eRes);
            }
        } else {
            remove(cRes.binaryPath.c_str());
        }
    } else {
        if (!jsonMode) reporter.reportCompilationError(cRes);
    }

    if (jsonMode) {
        reporter.reportJson(cRes, eRes);
    }

    return 0;
}