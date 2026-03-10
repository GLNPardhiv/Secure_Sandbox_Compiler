#include <iostream>

int main() {
    int a = 10;
    int b = 0;
    // This causes SIGFPE
    std::cout << (a / b) << std::endl; 
    return 0;
}