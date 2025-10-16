#include "benchmark.h"


struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

double benchmark(int iterations,
    const uint8_t* img, size_t width, size_t height, 
    float a, float b, float c, float gamma, uint8_t* result,
    gamma_func_ptr gamma_correct)
{
    gamma_correct(img, width, height, a, b, c, gamma, result);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    

    for(int i = 0; i < iterations; i++){
        gamma_correct(img, width, height, a, b, c, gamma, result);
    }
    

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    struct timespec res = diff(start, end);

    double s = (double) res.tv_sec + 1e-9 * res.tv_nsec;

    return s;
    
}
