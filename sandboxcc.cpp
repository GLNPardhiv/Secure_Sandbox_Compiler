#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <array>
#include <memory>
#include <fstream>
#include <vector>

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
    int threatScore = 0;

    // 1. FATAL THREATS (+100) -> Immediate Local Block
    std::vector<std::string> fatalKeywords = {
        "system(", "fork(", "exec", "asm(", "<sys/socket.h>", "popen(", "clone("
    };
    for (const auto& kw : fatalKeywords) {
        if (content.find(kw) != std::string::npos) {
            std::cout << "[!] ⚡ Fast-Fail: Dangerous keyword '" << kw << "' detected locally.\n";
            threatScore += 100;
            break; // No need to check further, it's already dead
        }
    }

    // 2. SUSPICIOUS FEATURES (+50) -> Force AI Scan
    // These aren't malicious by themselves, but hackers use them to obfuscate.
    std::vector<std::string> suspiciousFeatures = {
        "<unistd.h>",      // POSIX OS API
        "<string>",        // String manipulation (like "soc" + "ket")
        "vector",          // Data structures (padding)
        "(*",              // Function pointers (used to hide executions)
        "char ",           // Raw char arrays (used to hide commands)
        "fstream"          // File reading/writing
    };
    for (const auto& feature : suspiciousFeatures) {
        if (content.find(feature) != std::string::npos) {
            threatScore += 50;
        }
    }

    // --- DECISION ENGINE ---
    if (threatScore >= 100) {
        return false; // Fast-Fail: Block compilation
    } 
    else if (threatScore > 0) {
        skipAI = false; // Suspicious: Force Gemini Scan
        return true;    // Proceed to AI step
    } 
    else {
        std::cout << ">>> ⚡ Local Heuristic Analysis: Code looks simple & safe. Skipping AI.\n";
        skipAI = true;  // Score is 0: Pure boilerplate, Fast-Pass
        return true;    // Proceed to compile directly
    }
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