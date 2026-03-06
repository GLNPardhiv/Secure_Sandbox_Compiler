#include <iostream>
#include <string>
#include <cstring>
#include "compiler.h"
#include "sandbox.h"
#include "reporter.h"
#include "security.h" // Week 8

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

    CompileResult cRes = compiler.compile(sourceFile);
    ExecutionResult eRes;
    
    // Defaults
    eRes.exitCode = 0;
    eRes.signal = 0;
    eRes.timeout = false;

    if (cRes.success) {
        if (!jsonMode) std::cout << "Compilation: SUCCESS (Seccomp & Isolation Ready)\n";
        
        if (runRequested || jsonMode) {
            if (!jsonMode) {
                std::cout << ">>> Setting up File System Jail (Week 8)...\n";
                std::cout << ">>> Validating Memory Isolation...\n";
            }

            // WEEK 8: Setup File System Jail
            std::string jailedBinary = SecurityModule::setupJail(cRes.binaryPath);

            // Execute the jailed binary
            eRes = sandbox.execute(jailedBinary);
            
            // WEEK 8: Cleanup Jail
            SecurityModule::cleanupJail(jailedBinary);

            if (!jsonMode) {
                reporter.reportExecution(eRes);
                std::cout << "\n[Log] Security events logged to 'sandbox_exec.log'\n";
            }
        }
        remove(cRes.binaryPath.c_str());
    } else {
        if (!jsonMode) reporter.reportCompilationError(cRes);
    }

    if (jsonMode) {
        reporter.reportJson(cRes, eRes);
    }

    return 0;
}