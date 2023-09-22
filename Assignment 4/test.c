#include <stdio.h>
#include <unistd.h>
int main() {
    char ch;
    while(read(0, &ch, 1) > 0) {
        printf("%c\n", ch);
    }
    return 0;
}