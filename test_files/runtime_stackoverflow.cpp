#include <iostream>

void infinite_recursion(int depth) {
    // Each call uses stack memory for 'depth'
    char buffer[1024]; // 1KB per frame
    infinite_recursion(depth + 1);
}

int main() {
    infinite_recursion(0);
    return 0;
}