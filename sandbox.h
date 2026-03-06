#ifndef SANDBOX_H
#define SANDBOX_H

#include <string>

struct ExecutionResult {
    std::string output;
    std::string rawStatus;
    std::string interpretation;
    bool timeout;
    int exitCode;
    int signal;
};

class Sandbox {
public:
    ExecutionResult execute(const std::string& binaryPath);
private:
    void logEvent(const std::string& message);
};

#endif