#include <time.h>
#include <stdint.h>

typedef void (*gamma_func_ptr)(
    const uint8_t *img, size_t width, size_t height,
    float a, float b, float c,
    float gamma,
    uint8_t *result);

struct timespec diff(struct timespec start, struct timespec end);

double benchmark(int iterations,
    const uint8_t* img, size_t width, size_t height, 
    float a, float b, float c, float gamma, uint8_t* result,
    gamma_func_ptr gamma_correct);