#ifndef IMG_H
#define IMG_H
#include <png.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct pix {
    int r;
    int g;
    int b;
    int a;
} pix;

typedef struct pix_row {
    int lenght;
    pix *p;
} pix_row;

typedef struct image {

    png_structp png_ptr;
    png_infop info_ptr;
    int width;
    int height;
    FILE *fp;
} image;


image initialize_png(char *title, char *filename, int width, int height);
void write_image(image * img, pix_row * img_array);
void finish_image(image * img);

#endif
