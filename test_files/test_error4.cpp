#include <iostream>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

int main() {
    cout << "Attempting to fork a new process..." << endl;
    
    // This system call is BANNED by your compiler.cpp payload
    pid_t pid = fork(); 
    
    if (pid < 0) {
        cout << "Fork failed (Handled by OS?)" << endl;
    } else {
        cout << "Fork succeeded! (Sandbox Failed)" << endl;
    }
    
    return 0;
}