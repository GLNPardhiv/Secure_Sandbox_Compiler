#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <array>
#include <memory>
#include <fstream> 

#include "compiler.h"
#include "sandbox.h"
#include "reporter.h"
#include "security.h"

// --- Helper: Call Python Script ---
// Modes: "analyze", "compile_error", "runtime_error"
std::string callAI(const std::string& mode, const std::string& file, const std::string& extraArg = "") {
    std::string command = "python3 risk_analyzer.py " + mode + " " + file;
    if (!extraArg.empty()) {
        command += " " + extraArg;
    }
    
    std::array<char, 2048> buffer;
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "AI System Failure";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

bool quickHeuristicCheck(const std::string& sourcePath, bool& skipAI) {
    std::ifstream f(sourcePath);
    if (!f.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    
    // 1. FAST FAIL: Dangerous Keywords (Block immediately)
    // If these exist, we don't even need AI to tell us it's bad.
    const char* badKeywords[] = {"fork(", "system(", "exec(", "socket(", "popen(", "clone("};
    for (const char* kw : badKeywords) {
        if (content.find(kw) != std::string::npos) {
            std::cout << "[!] ⚡ Fast-Fail: Dangerous keyword '" << kw << "' detected locally.\n";
            return false; // Block immediately
        }
    }

    // 2. FAST PASS: Simple Code (Skip AI)
    // If the code is small and only uses standard IO, trust the Sandbox to catch runtime errors.
    // This makes "Hello World" instant.
    bool hasIoStream = content.find("#include <iostream>") != std::string::npos;
    bool hasVector = content.find("#include <vector>") != std::string::npos;
    
    // If code is short (< 300 chars) and doesn't have complex headers like <unistd.h>
    if (content.length() < 300 && 
        content.find("#include <unistd.h>") == std::string::npos &&
        content.find("#include <sys/") == std::string::npos) {
        
        std::cout << ">>> ⚡ Local Heuristic Analysis: Code looks simple & safe. Skipping AI.\n";
        skipAI = true; // Tell main to skip the API call
        return true;   // Allow compilation
    }

    // 3. AMBIGUOUS: Code is complex or long.
    skipAI = false; // Must call AI
    return true;    // Proceed to AI check
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./sandboxcc <file> [--run]\n";
        return 1;
    }

    std::string sourceFile = argv[1];
    bool runRequested = false;
    if (argc > 2 && strcmp(argv[2], "--run") == 0) runRequested = true;

    Compiler compiler;
    Sandbox sandbox;
    Reporter reporter;

    // ---------------------------------------------------------
    // PHASE 1: PRE-EXECUTION AI RISK ANALYSIS (Week 9)
    // ---------------------------------------------------------
        // ---------------------------------------------------------
    // PHASE 1: PRE-EXECUTION RISK ANALYSIS (Week 9 + 11 Optimization)
    // ---------------------------------------------------------
    
    bool skipAI = false;
    
    // Tier 1: Local Check
    if (!quickHeuristicCheck(sourceFile, skipAI)) {
        std::cout << "\n[!] 🛑 BLOCKED BY LOCAL SECURITY FILTER.\n";
        return 1;
    }

    // Tier 2: AI Check (Only if not skipped)
    if (!skipAI) {
        std::cout << ">>> 🧠 Invoking AI Risk Analyzer (Complex Code Detected)... ";
        std::string riskJson = callAI("analyze", sourceFile);
        
        if (riskJson.find("\"is_safe\": false") != std::string::npos) {
            std::cout << "\n\n[!] 🛑 BLOCKED BY AI SECURITY POLICY\n";
            std::cout << "Analysis: " << riskJson << "\n";
            return 1;
        }
        std::cout << "✅ Safe.\n";
    }

    // ---------------------------------------------------------
    // PHASE 2: COMPILATION (Weeks 1-6)
    // ---------------------------------------------------------
    CompileResult cRes = compiler.compile(sourceFile);

    if (!cRes.success) {
        reporter.reportCompilationError(cRes);
        
        // --- WEEK 10: AI COMPILER ERROR EXPLANATION ---
        std::cout << "\n>>> 🤖 AI Tutor (Compiler Help):\n";
        // Pass the error log file path as the 3rd argument
        std::string explanation = callAI("compile_error", sourceFile, "compile_errors.txt");
        std::cout << explanation << "\n";
        return 1;
    }

    // ---------------------------------------------------------
    // PHASE 3: EXECUTION (Weeks 7-8)
    // ---------------------------------------------------------
    if (runRequested) {
        std::string jailedBinary = SecurityModule::setupJail(cRes.binaryPath);
        ExecutionResult eRes = sandbox.execute(jailedBinary);
        SecurityModule::cleanupJail(jailedBinary);
        remove(cRes.binaryPath.c_str());

        reporter.reportExecution(eRes);

        // --- WEEK 10: AI RUNTIME ERROR EXPLANATION ---
        if (eRes.exitCode != 0 || eRes.signal != 0) {
            std::cout << "\n>>> 🤖 AI Tutor (Runtime Crash Analysis):\n";
            std::string signalStr = std::to_string(eRes.signal);
            std::string explanation = callAI("runtime_error", sourceFile, signalStr);
            std::cout << explanation << "\n";
        }
    } else {
        remove(cRes.binaryPath.c_str());
        std::cout << "Compilation successful (Binary removed).\n";
    }

    return 0;
}