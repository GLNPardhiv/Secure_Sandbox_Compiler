#include <iostream>
// The word 'system' here triggers the AI check
void run_system() { 
    std::cout << "This looks suspicious because of the function name." << std::endl;
}

int main() {
    run_system();
    return 0;
}