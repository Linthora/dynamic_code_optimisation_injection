#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/prctl.h>


/* #include <sys/types.h>
#include <unistd.h> */

// à opti en live en speed_ext
int exponentiation_long_long(int x, int y) {

    if(y < 0) {
        printf("Error: y must be positive\n");
        return -1;
    }

    if(y == 0) {
        return 1;
    }

    int res = 1;
    for(int i = 0; i < y; i++) {
        res *= x;
    }
    return res;
}


int foo(int * i) {
    for(int j = 0; j < *i; j++)
        printf("FOO1\n");
    printf("FOO3\n");
    int res = *i+4;
    // modify i to be 666
    *i = 666;
    return res;
}

void answer() {
    //long long res;
    for(int j = 0; j < 10000; j++)
        exponentiation_long_long(3, 100000);

    // printf("getpagesize: %d\n", getpagesize());
    printf("exponentiation_long_long: %d\n", exponentiation_long_long(3, 100000));
}

void* posixx_al_call() {
    size_t allign = getpagesize();
    void* ptr;
    size_t size = 145;
    int res = posix_memalign(&ptr, allign, size);
    return ptr;
}

// idée -> csv pour indiquer le temps pris par chaque itération et plot le résultat. (python)
int main(int argc, char *argv[]) {
    
    prctl(PR_SET_PTRACER,PR_SET_PTRACER_ANY);
    //foo(42);

    //exit(0);

    long long nb_iter = __LONG_LONG_MAX__;

    if(argc > 1) {
        nb_iter = atoi(argv[1]);
    }

    clock_t start, stop;
    //float data_time_taken[nb_iter];

    for(long long i = 0; i < nb_iter; i++) {
        start = clock();
        //for(int j = 0; j < 10000; j++)
        answer();
        stop = clock();
        float time_taken = ((float)stop - (float)start) / CLOCKS_PER_SEC;
        //data_time_taken[i] = time_taken;
        //printf("Time taken: %f\n", data_time_taken[i]);
        printf("Time taken: %f\n", time_taken);
    }
    // write in csv: TODO latter
    

    return 0;
}