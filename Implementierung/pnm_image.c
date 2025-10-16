#include "pnm_image.h"
#include <stdio.h>
#include <stdlib.h>


PNMImage *load_ppm_p6(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Fehler: Fehler beim Oeffnen der PPM-Datei %s\n", filename);
        return NULL;
    }

    char magic[3];
    if (!fgets(magic, sizeof(magic), file))
    {
        fprintf(stderr, "Fehler: Fehler beim Lesen von \"magic number\"\n");
        fclose(file);
        return NULL; 
    }

    // check if file is P6
    if (magic[0] != 'P' || magic[1] != '6')
    {
        fprintf(stderr, "Fehler: Ist kein P6 File-Typ.\n");
        fclose(file);
        return NULL; 
    }

    // read width, height, max_val
    size_t width, height;
    int max_val;
    if (fscanf(file, "%zu %zu %d", &width, &height, &max_val) != 3)
    {
        fprintf(stderr, "Fehler: Fehler beim Lesen von Breite/Hoehe/max_val\n");
        fclose(file);
        return NULL;
    }

    // if > 255, not correct num
    if (max_val > 255)
    {
        fprintf(stderr, "Fehler: Nur 8-Bit-PPM (max_val <= 255) werden unterstuetzt.\n");
        fclose(file);
        return NULL;
    }

    // read the newline after max_value to set reader ready to reading of pixels
    fgetc(file);

    // allocate memort for image
    PNMImage *image = (PNMImage *)malloc(sizeof(PNMImage));
    if (!image)
    {
        fprintf(stderr, "Fehler: Fehler: Kein Speicher fuer PNMImage.\n");
        fclose(file);
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->max_val = max_val;

    // allocate data of image (width * height) Pixel, 3 Byte pro value
    size_t num_pixels = width * height;
    size_t buffer_size = num_pixels * 3;
    image->data = (uint8_t *)malloc(buffer_size);
    if (!image->data)
    {
        fprintf(stderr, "Fehler: Fehler: Kein Speicher fuer Bilddaten.\n");
        free(image);
        fclose(file);
        return NULL;
    }

    //  read image data
    size_t read_bytes = fread(image->data, 1, buffer_size, file);
    if (read_bytes != buffer_size)
    {
        fprintf(stderr, "Fehler: Fehler beim Einlesen der PNM-Daten.\n");
        free(image->data);
        free(image);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return image;
}

PNMImage *create_grayscale_pgm(PNMImage *img){
    PNMImage *image = (PNMImage *)malloc(sizeof(PNMImage));
    if (!image)
    {
        fprintf(stderr, "Fehler: Kein Speicher fuer PGMImage.\n");
        return NULL;
    }

    // maybe for not having copypaste make a functuion??
    image->width = img->width;
    image->height = img->height;
    image->max_val = img->max_val;

    // allocate data of image (width * height) Pixel, 3 Byte pro value
    size_t num_pixels = img->width * img->height;
    image->data = (uint8_t *)malloc(num_pixels);
    if (!image->data)
    {
        fprintf(stderr, "Fehler: Kein Speicher fuer PGM Bilddaten.\n");
        free(image);
        return NULL;
    }
    return image;
}

void writePGMP5(const char *filename, const PNMImage *img)
{
    // wb - binary write mode
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "Fehler: Kann die Datei zum Schreiben nicht oefenen\n");
        return;
    }

    // P5 \n width height \n max_val
    fprintf(fp, "P5\n%zu %zu\n%d\n", img->width, img->height, img->max_val);

    size_t num_pixels = img->width * img->height;
    fwrite(img->data, sizeof(uint8_t), num_pixels, fp);

    fclose(fp);

    return;
}