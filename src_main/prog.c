#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/prctl.h>


// à opti en live en speed_ext
long long exp_here(long long x, long long y) {

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

void answer() {
    printf("Hello, exp(3,100) = %lli\n", exp_here(3, 10000));
}

// idée -> csv pour indiquer le temps pris par chaque itération et plot le résultat. (python)
int main(int argc, char *argv[]) {
    long long nb_iter = 5000000;
    
    if(argc > 1) {
        nb_iter = atoi(argv[1]);
    }

    clock_t start, stop;
    float data_time_taken[nb_iter];

    for(long long i = 0; i < nb_iter; i++) {
        start = clock();
        answer();
        stop = clock();
        float time_taken = ((float)stop - (float)start) / CLOCKS_PER_SEC;
        data_time_taken[i] = time_taken;
        printf("Time taken: %f\n", data_time_taken[i]);
    }
    // write in csv: TODO latter
    

    return 0;
}