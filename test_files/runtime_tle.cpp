int main() {
    // Busy wait for 5 seconds
    // Your sandbox limit is probably 1 or 2 seconds
    long long i = 0;
    while(i < 5000000000LL) {
        i++;
    }
    return 0;
}