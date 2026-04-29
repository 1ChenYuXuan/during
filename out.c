
#include <stdio.h>

typedef struct A {
    int A_a;
    int A_b;

} A;

int main() {
    int a = 1;
    int b = 1;
    while (a < 10000) {
        a = a + 1;
        b = b + 1;
        printf("%d\n", a);
    }
    return 0;
}