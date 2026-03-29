#include <unistd.h>
int main() {
    unlink("/etc/passwd"); // System call to delete a file
    return 0;
}