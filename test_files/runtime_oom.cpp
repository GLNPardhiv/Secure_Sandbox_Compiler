#include <vector>
#include <iostream>

int main() {
    std::vector<int*> pointers;
    while (true) {
        // Allocate 10MB chunks
        int* ptr = new int[2500000]; 
        pointers.push_back(ptr);
        // Don't free it!
    }
    return 0;
}