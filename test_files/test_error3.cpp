#include<bits/stdc++.h>
using namespace std;

//trying to access address of a nullpointer location, program should terminate
int main() {
    int* p = nullptr;
    *p = 5;
}
