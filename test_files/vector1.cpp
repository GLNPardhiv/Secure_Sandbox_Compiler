#include <iostream>
#include <vector>

int main() {
    // Needs SCMP_SYS(brk) or mmap
    std::vector<int> v;
    for(int i=0; i<100; i++) v.push_back(i);
    
    // Needs SCMP_SYS(write)
    std::cout << "Vector size: " << v.size() << std::endl;
    
    return 0; // Needs SCMP_SYS(exit_group)
}