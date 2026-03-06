#include <iostream>
using namespace std;

int main() {
    // Disable sync to avoid certain syscall overheads
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    cout << "Hello, World!" << "\n"; // Use \n instead of endl to avoid forced flushing
    return 0;
}