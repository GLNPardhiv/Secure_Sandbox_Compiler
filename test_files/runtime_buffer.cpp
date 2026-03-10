#include <cstring>
#include <iostream>

int main() {
    char buffer[10];
    // Writing 100 bytes into a 10-byte buffer
    // This corrupts memory
    std::strcpy(buffer, "This string is way too long for the buffer!");
    std::cout << buffer << std::endl;
    return 0;
}