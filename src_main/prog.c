#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/prctl.h>


/* #include <sys/types.h>
#include <unistd.h> */

// à opti en live en speed_ext
long long exponentiation_long_long(long x, long long y) {

    if(y < 0) {
        printf("Error: y must be positive\n");
        return -1;
    }

    long long res = 1;
    for(long long i = 0; i < y; i++) {
        res *= x;
    }
    return res;
}

void answer(long long i) {
    long long res = exponentiation_long_long(3, 10000);
    if(i % 1000 == 0)
        printf("Hello, exp(3,100) = %lli\n", res);
}

void foo() {
    printf("FOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
}

// idée -> csv pour indiquer le temps pris par chaque itération et plot le résultat. (python)
int main(int argc, char *argv[]) {

    long long nb_iter = __LONG_LONG_MAX__;

    if(argc > 1) {
        nb_iter = atoi(argv[1]);
    }

    clock_t start, stop;
    //float data_time_taken[nb_iter];

    for(long long i = 0; i < nb_iter; i++) {
        start = clock();
        for(int j = 0; j < 1000; j++)
            answer(j);
        stop = clock();
        float time_taken = ((float)stop - (float)start) / CLOCKS_PER_SEC;
        //data_time_taken[i] = time_taken;
        //printf("Time taken: %f\n", data_time_taken[i]);
        printf("Time taken: %f\n", time_taken);
    }
    // write in csv: TODO latter
    

    return 0;
}