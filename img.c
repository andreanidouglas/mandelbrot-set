#include "img.h"


image initialize_png(char *title, char *filename, int width, int height)
{
    image ret;
    FILE *fp = NULL;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
	fprintf(stderr, "could not create %s", filename);
	exit(EXIT_FAILURE);
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
						  NULL,
						  NULL,
						  NULL);

    if (png_ptr == NULL) {
	fprintf(stderr, "Could not create write struct. libpng\n");
    }


    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
	fprintf(stderr, "Could not create info struct. libpng\n");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
	fprintf(stderr, "Error during png creation\n");
    }

    png_init_io(png_ptr, fp);

    //Write header
    png_set_IHDR(png_ptr,
		 info_ptr,
		 width,
		 height,
		 8,
		 PNG_COLOR_TYPE_RGB,
		 PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    if (title != NULL) {
	png_text title_text;
	title_text.compression = PNG_TEXT_COMPRESSION_NONE;
	title_text.key = "Title";
	title_text.text = title;
	png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    ret.png_ptr = png_ptr;
    ret.info_ptr = info_ptr;
    ret.width = width;
    ret.height = height;
    ret.fp = fp;
    return ret;
}

void write_image(image * img, pix_row * img_array)
{

    png_bytep row = NULL;
    int x = 0, y = 0;
    row = (png_bytep) malloc(3 * img->width * sizeof(png_byte));
    for (y = 0; y < img->height; y++) {
	for (x = 0; x < img->width; x++) {
	    row[x * 3 + 0] = img_array[x].p[y].r;	//red
	    row[x * 3 + 1] = img_array[x].p[y].g;	// green
	    row[x * 3 + 2] = img_array[x].p[y].b;	// blue
	}
	png_write_row(img->png_ptr, row);
    }
    png_write_end(img->png_ptr, img->info_ptr);
    free(row);
}

void finish_image(image * img)
{
    fclose(img->fp);
    png_free_data(img->png_ptr, img->info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&img->png_ptr, NULL);

}
