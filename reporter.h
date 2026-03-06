#ifndef REPORTER_H
#define REPORTER_H

#include "compiler.h"
#include "sandbox.h"
#include <string>

class Reporter {
public:
    void reportCompilationError(const CompileResult& res);
    void reportExecution(const ExecutionResult& res);
    void reportJson(const CompileResult& cRes, const ExecutionResult& eRes); // For Web
};

#endif