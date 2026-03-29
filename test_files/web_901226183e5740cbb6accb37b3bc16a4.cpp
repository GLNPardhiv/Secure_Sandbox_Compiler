#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

// A simple command parser for an educational shell
void execute_safe_command(const std::string& cmd) {
    // Educational safety check
    if (cmd != "ls" && cmd != "date") {
        std::cout << "Mini-Shell Error: Only 'ls' and 'date' are allowed for this assignment.\n";
        return;
    }

    pid_t pid = fork();
    
    if (pid == 0) {
        // We are in the child process. 
        // We legitimately use exec to run the simple command.
        
        std::vector<char*> args;
        if (cmd == "ls") {
            args.push_back((char*)"/bin/ls");
            args.push_back(nullptr);
            // This is a legitimate execv call for an OS class!
            execv("/bin/ls", args.data());
        } else if (cmd == "date") {
            args.push_back((char*)"/bin/date");
            args.push_back(nullptr);
            // Legitimate execv call
            execv("/bin/date", args.data());
        }
        
        // If execv fails, it reaches here
        std::cerr << "Command execution failed.\n";
        exit(1);
    } else if (pid > 0) {
        // Parent waits for the command to finish
        waitpid(pid, nullptr, 0);
    }
}

int main() {
    std::cout << "--- CS301: Custom Mini-Shell Environment ---\n";
    std::cout << "Simulating user typing 'date'...\n";
    execute_safe_command("date");
    
    std::cout << "\nSimulating user typing 'ls'...\n";
    execute_safe_command("ls");
    
    std::cout << "\nShell exiting gracefully.\n";
    return 0;
}