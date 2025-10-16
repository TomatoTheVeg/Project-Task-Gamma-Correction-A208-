#include <stddef.h>
#include <stdint.h>


typedef struct {
    size_t width;
    size_t height;
    int max_val;
    uint8_t *data; 
} PNMImage; //sometimes PPM and PBM are called PNM, so we call it also so


PNMImage *load_ppm_p6(const char *filename);
PNMImage *create_grayscale_pgm(PNMImage *img);
void writePGMP5(const char *filename, const PNMImage *img);

