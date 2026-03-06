#include <iostream>
#include <cstdlib>

using namespace std;

int main() {
    cout << "Attempting to execute shell command 'ls'..." << endl;
    
    // This tries to launch a shell. Your injected code blocks execve/clone.
    system("ls -la");
    
    cout << "If you see this, the shell command finished (Sandbox Failed)." << endl;
    return 0;
}