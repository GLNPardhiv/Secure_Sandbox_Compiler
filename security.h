#ifndef SECURITY_H
#define SECURITY_H

#include <string>

class SecurityModule {
public:
    // Generates the Seccomp-BPF code to be injected
    static std::string getSeccompPreamble();
    
    // Sets up the file system jail (creates dir, moves binary)
    static std::string setupJail(const std::string& originalBinary);
    
    // Cleans up the jail after execution
    static void cleanupJail(const std::string& jailPath);
};

#endif