#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/prctl.h>


/* #include <sys/types.h>
#include <unistd.h> */

/**
 * The function that we want to optimize
 * Function to compute x^y
 * But here, it's not done in a fast way
*/
long long exponentiation_long_long(long long x, long long y) {

    if(y < 0) {
        printf("Error: y must be positive\n");
        return -1;
    }

    if(y == 0) {
        return 1;
    }

    long long res = 1;
    for(long long i = 0; i < y; i++) {
        res *= x;
    }
    return res;
}

// Test function for challenge 2
int foo(int * i) {
    for(int j = 0; j < *i; j++)
        printf("FOO1\n");
    printf("FOO3\n");
    int res = *i+4;
    // modify i to be 666
    *i = 666;
    return res;
}

// Function used to monitorize the time taken by the function to optimize
void answer() {
    for(int j = 0; j < 1000000; j++)
        exponentiation_long_long(3, 1000);

    printf("exponentiation_long_long: %lli\n", exponentiation_long_long(3, 1000));
}

/**
 * Main
*/
int main() {

    // allow to the process to be traced    
    prctl(PR_SET_PTRACER,PR_SET_PTRACER_ANY);

    // get the number of iteration to do. Set to __LONG_LONG_MAX__ to be able to see the optimization
    long long nb_iter = __LONG_LONG_MAX__;

    clock_t start, stop;

    // the main loop
    for(long long i = 0; i < nb_iter; i++) {
        start = clock();
        answer();
        stop = clock();
        float time_taken = ((float)stop - (float)start) / CLOCKS_PER_SEC;
        printf("Time taken: %f\n", time_taken);
    }
    
    return 0;
}