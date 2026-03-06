#include "sandbox.h"
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <fstream>
#include <ctime>
#include <iostream>
#include <cstring>

// Helper to log events to a file
void Sandbox::logEvent(const std::string& message) {
    std::ofstream logFile("sandbox_exec.log", std::ios_base::app);
    std::time_t now = std::time(nullptr);
    char* dt = std::ctime(&now);
    if (dt) dt[strlen(dt)-1] = '\0'; // Remove newline
    logFile << "[" << (dt ? dt : "Time") << "] " << message << "\n";
}

ExecutionResult Sandbox::execute(const std::string& binaryPath) {
    ExecutionResult result;
    result.timeout = false;
    result.exitCode = -1;
    result.signal = 0;

    logEvent("------------------------------------------------");
    logEvent("Initiating Sandboxed Execution for: " + binaryPath);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        logEvent("Error: Failed to create output pipe.");
        return result;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // --- CHILD PROCESS ---
        close(pipefd[0]); // Close read end
        
        // Redirect stdout/stderr
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) _exit(1);
        if (dup2(pipefd[1], STDERR_FILENO) == -1) _exit(1);
        close(pipefd[1]); // Close write end after dup

        // 1. CPU Time Limit (1s soft, 2s hard)
        struct rlimit cpuLimit = {1, 2};
        setrlimit(RLIMIT_CPU, &cpuLimit);

        // 2. Memory Limit (64MB)
        struct rlimit memLimit = {64 * 1024 * 1024, 64 * 1024 * 1024};
        setrlimit(RLIMIT_AS, &memLimit);

        // 3. Process Limit (Prevent Fork Bombs)
        // We set this to 0 to prevent the child from creating NEW processes.
        struct rlimit nprocLimit = {0, 0};
        setrlimit(RLIMIT_NPROC, &nprocLimit);

        // 4. File Size Limit (1MB)
        struct rlimit fsizeLimit = {1024 * 1024, 1024 * 1024};
        setrlimit(RLIMIT_FSIZE, &fsizeLimit);

        // 5. Disable Core Dumps
        struct rlimit coreLimit = {0, 0};
        setrlimit(RLIMIT_CORE, &coreLimit);

        // Execute
        execl(binaryPath.c_str(), binaryPath.c_str(), NULL);
        
        // If execl fails
        _exit(1); 
    }

    // --- PARENT PROCESS ---
    close(pipefd[1]); // Close write end immediately

    logEvent("Child process spawned with PID: " + std::to_string(pid));
    
    const int TIMEOUT_MS = 2000;
    int elapsed = 0;
    int status;
    bool killedByTimeout = false;

    while (true) {
        pid_t res = waitpid(pid, &status, WNOHANG);
        
        if (res == pid) {
            // Child exited naturally
            break; 
        }
        
        if (res == -1) {
            // Error in waitpid
            break;
        }

        if (elapsed >= TIMEOUT_MS) {
            logEvent("Timeout reached. Killing process.");
            kill(pid, SIGKILL);
            killedByTimeout = true;
            result.timeout = true;
            // CRITICAL: Must wait for the kill to finish
            waitpid(pid, &status, 0); 
            break;
        }
        
        usleep(100000); // 100ms
        elapsed += 100;
    }

    // Read remaining output
    char buffer[128];
    ssize_t n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        result.output += buffer;
    }
    close(pipefd[0]);

    // Interpret Status
    if (WIFSIGNALED(status)) {
        result.signal = WTERMSIG(status);
        result.rawStatus = "Terminated by signal " + std::to_string(result.signal);
        logEvent("Process terminated by signal: " + std::to_string(result.signal));

        if (result.signal == SIGSYS) { 
            result.interpretation = "SECURITY VIOLATION: Blocked system call attempted";
            result.rawStatus += " (SIGSYS)";
        }
        else if (result.signal == SIGSEGV) result.interpretation = "Invalid memory access";
        else if (result.signal == SIGXCPU) result.interpretation = "CPU time limit exceeded";
        else if (result.signal == SIGXFSZ) result.interpretation = "File output size limit exceeded";
        else if (result.signal == SIGKILL && killedByTimeout) result.interpretation = "Time limit exceeded";
        else result.interpretation = "Abnormal termination";
    } 
    else if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
        result.rawStatus = "Completed";
        logEvent("Process exited normally with code: " + std::to_string(result.exitCode));
    }

    return result;
}