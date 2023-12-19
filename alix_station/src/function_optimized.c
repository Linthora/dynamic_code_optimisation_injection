#include <stdio.h>

// function to optimized
int fast_exponentiation_long_long(int x, int y) {
    int res = x + y;
    return res;
    /* // fast exponentiation
    if(y < 0) {
        printf("Error: y must be positive\n");
        return -1;
    }

    long long res = 1;
    while(y > 0) {
        if(y % 2 == 1) {
            res *= x;
        }
        x *= x;
        y /= 2;
    }
    return res; */
}


int yolo(int x) {
    return x + 1;
}

int main() {
    
}