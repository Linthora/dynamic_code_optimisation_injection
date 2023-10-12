#include <stdio.h>
#include <time.h>




// à opti en live en speed_ext
int exp(long long x, long long y) {

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
    printf("Hello, exp(10,30) = %ll\n", exp(10, 30));
}

// idée -> csv pour indiquer le temps pris par chaque itération et plot le résultat. (python)
int main(int argc, char *argv[]) {
    int nb_iter = 50000;
    
    if(argc > 1) {
        nb_iter = atoi(argv[1]);
    }

    clock_t start, stop;
    float data_time_taken[nb_iter];

    for(int i = 0; i < nb_iter; i++) {
        start = clock();
        answer();
        stop = clock();
        float time_taken = ((float)stop - (float)start) / CLOCKS_PER_SEC;
        printf("Time taken: %f\n", time_taken);
        data_time_taken[i] = time_taken;
    }
    // write in csv: TODO latter
    

    return 0;
}