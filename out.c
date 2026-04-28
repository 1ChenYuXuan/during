
#include <stdio.h>

int main() {
    int a = 1;
    while (a < 10000) {
        a = a + 1;
        printf("%d\n", a);
    }
    return 0;
}