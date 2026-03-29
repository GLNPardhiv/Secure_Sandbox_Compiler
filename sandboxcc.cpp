#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <array>
#include <memory>
#include <fstream>
#include <vector>
#include <unistd.h> 

#include "compiler.h"
#include "sandbox.h"
#include "reporter.h"
#include "security.h"

// --- Helper: Call Python Script ---
// Modes: "analyze", "compile_error", "runtime_error"
std::string callAI(const std::string& mode, const std::string& file, const std::string& extraArg = "") {
    // FORCE execution from current directory and capture stderr to prevent silent Python crashes
    std::string command = "cd $(pwd) && python3 risk_analyzer.py " + mode + " " + file;
    if (!extraArg.empty()) {
        command += " " + extraArg;
    }
    command += " 2>&1"; 
    
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

    // 1. FATAL THREATS (+100) -> Immediate Local Block (Zero Latency)
    // ONLY block things that have absolutely zero legitimate use in a basic academic sandbox
    std::vector<std::string> fatalKeywords = {
        "asm(", "__asm__", "<sys/socket.h>", "<netinet/in.h>", "ptrace("
    };
    for (const auto& kw : fatalKeywords) {
        if (content.find(kw) != std::string::npos) {
            std::cout << "[!] ⚡ Fast-Fail: Fatal hardware/network keyword '" << kw << "' detected locally.\n";
            return false; // Block immediately without AI
        }
    }

    // 2. SUSPICIOUS FEATURES (+50) -> Force AI Scan (Contextual Analysis)
    // We let the AI read the context to decide if it's safe or malicious.
    std::vector<std::string> suspiciousFeatures = {
        "exec(",           // Could be malware, or OS homework
        "system(",         // Could be malware, or harmless shell script
        "fork(",           // Could be a fork bomb, or legitimate threading
        "clone(",          // Advanced threading
        "popen(",          // Piping
        "<unistd.h>",      // POSIX API
        "<pthread.h>",     // Threading
        "<string>",        // String manipulation (often used to hide commands)
        "(*",              // Function pointers (used to obfuscate execution)
        "char "            // Raw char arrays (used to hide hex commands)
    };
    
    for (const auto& feature : suspiciousFeatures) {
        if (content.find(feature) != std::string::npos) {
            threatScore += 50;
        }
    }

    // --- DECISION ENGINE ---
    if (threatScore > 0) {
        // Suspicious: Force Gemini Scan to analyze context
        skipAI = false; 
        return true;    
    } 
    else {
        // Score is 0: Pure boilerplate (iostream, basic math). 
        std::cout << ">>> ⚡ Local Heuristic Analysis: Code looks simple & safe. Skipping AI.\n";
        skipAI = true;  
        return true;    
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

    bool skipAI = false;
    
    // Tier 1: Local Check
    if (!quickHeuristicCheck(sourceFile, skipAI)) {
        std::cout << "\n[!] 🛑 BLOCKED BY LOCAL SECURITY FILTER.\n";
        return 1;
    }

    // Tier 2: AI Check
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

    // Tier 3: COMPILATION
    CompileResult cRes = compiler.compile(sourceFile);

    if (!cRes.success) {
        reporter.reportCompilationError(cRes);
        std::cout << "\n>>> 🤖 AI Tutor (Compiler Help):\n";
        std::string explanation = callAI("compile_error", sourceFile, "compile_errors.txt");
        std::cout << explanation << "\n";
        return 1;
    }

    // Tier 4: EXECUTION & AI TUTOR
    if (runRequested) {
        std::string jailedBinary = SecurityModule::setupJail(cRes.binaryPath);
        ExecutionResult eRes = sandbox.execute(jailedBinary);
        SecurityModule::cleanupJail(jailedBinary);
        remove(cRes.binaryPath.c_str());

        reporter.reportExecution(eRes);

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