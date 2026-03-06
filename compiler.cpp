#include "compiler.h"
#include "security.h" // Week 8 Integration
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

std::string Compiler::injectSecurityPreamble(const std::string& sourceFile) {
    std::string tempFileName = "temp_sandboxed.cpp";
    std::ofstream outFile(tempFileName);
    std::ifstream inFile(sourceFile);

    // 1. Inject Week 8 Security Module Preamble
    outFile << SecurityModule::getSeccompPreamble() << "\n";

    // 2. Append User Code
    outFile << inFile.rdbuf();

    outFile.close();
    return tempFileName;
}

CompileResult Compiler::compile(const std::string& sourceFile) {
    CompileResult result;
    result.binaryPath = "./a.out"; 

    std::string readyFile = injectSecurityPreamble(sourceFile);

    int logFd = open("compile_errors.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    pid_t pid = fork();

    if (pid == 0) {
        dup2(logFd, STDERR_FILENO);
        close(logFd);
        execlp("g++", "g++", readyFile.c_str(), "-o", result.binaryPath.c_str(), NULL);
        _exit(1);
    }

    close(logFd);
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        result.success = true;
    } else {
        result.success = false;
        std::ifstream logReader("compile_errors.txt");
        result.errorOutput.assign((std::istreambuf_iterator<char>(logReader)),
                                   std::istreambuf_iterator<char>());
    }

    remove(readyFile.c_str());
    return result;
}