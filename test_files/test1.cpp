#include <iostream>
#include <vector>

using namespace std;

int main() {
    cout << "Initializing Secure Sandbox Test..." << endl;
    
    vector<int> numbers = {1, 2, 3, 4, 5};
    int sum = 0;
    for(int n : numbers) {
        sum += n;
    }
    
    cout << "Calculation complete. Sum is: " << sum << endl;
    return 0;
}