#ifndef COMPILER_H
#define COMPILER_H

#include <string>

struct CompileResult {
    bool success;
    std::string binaryPath;
    std::string errorOutput; // Captures compiler stderr
};

class Compiler {
public:
    CompileResult compile(const std::string& sourceFile);
private:
    std::string injectSecurityPreamble(const std::string& sourceFile);
};

#endif