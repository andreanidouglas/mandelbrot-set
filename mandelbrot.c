#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


#include "../libpng/img.h"
#include "config.h"

typedef struct mandel_field_t {
    int h, w;
    int *arr;
    pthread_mutex_t mut;
} mandel_field_t;

typedef struct mandel_t {
    int x, y;
    int bail_out;
    int max_iter;
    mandel_field_t field;
} mandel_t;

typedef struct inner {
    pthread_mutex_t mut;
} inner;

typedef struct buffer {
    sem_t empty;
    sem_t full;
    pthread_mutex_t mut;
    int pos;
    mandel_t **pix;
    inner *in;
} buffer;



double map(int value, int min, int max, int map_min, int map_max)
{
    double R = (double) (map_max - map_min) / (double) (max - min);
    double y = map_min + (value * R) + R;
    return y;
}


int mandelbrot_pix(double x, double y, int bail_out, int max_iter)
{
    int k;
    double real, imaginary, x2, y2;

    x2 = x;
    y2 = y;

    // iterate a^2 - b^2 + 2ab
    for (k = 0; k < max_iter; k++) {
	// a^2 - b^2
	real = x * x - y * y;
	// 2ab
	imaginary = 2 * x * y;

	// x = real + c
	x = real + x2;
	// y = imaginary + c
	y = imaginary +y2;

	if ((x * x + y * y) > bail_out) {
	    return k;
	}
    }
    return max_iter;
}

void init_buffer(buffer * buf)
{
    sem_init(&buf->empty, 0, 0);
    sem_init(&buf->full, 0, BUFFER_SIZE);
    buf->in = malloc(sizeof(inner));
    buf->pos = 0;
    buf->pix = malloc(sizeof(mandel_t) * BUFFER_SIZE);

    for (int i = 0; i < BUFFER_SIZE; i++) {
	buf->pix[i] = NULL;
    }
}

void mandel_field_setXY(mandel_field_t m, int x, int y, int value)
{
    pthread_mutex_lock(&m.mut);
    m.arr[x * m.h + y] = value;
    pthread_mutex_unlock(&m.mut);
}

void *mandel_consumer(void *buff)
{
    buffer *buf = (buffer *) buff;
    mandel_t *pix;
    int count = 0;
    double xPrime, yPrime;

    while (1) {

	int tw = sem_trywait(&buf->empty);
	if (tw == 0) {
	    pthread_mutex_lock(&buf->mut);

	    buf->pos = buf->pos - 1;
	    pix = buf->pix[buf->pos];
	    count++;
	    sem_post(&buf->full);


	    xPrime = map(pix->x, 0, pix->field.w, -2, 2);
	    yPrime = map(pix->y, 0, pix->field.h, -2, 2);

	    int k = mandelbrot_pix(xPrime, yPrime, pix->bail_out,
				   pix->max_iter);
	    mandel_field_setXY(pix->field, pix->x, pix->y, k);
	    pthread_mutex_unlock(&buf->mut);
	} else {
	    int rc = pthread_mutex_trylock(&buf->in->mut);
	    if (rc == 0) {
		printf("\ngot lock");
		pthread_mutex_unlock(&buf->in->mut);
		pthread_exit(NULL);
	    }
	}
    }
}

void mandelbrot(int *field, int w, int h, int bail_out, int max_iter,
		int thread_count)
{
    double xPrime, yPrime;
    int x, y, k;

    buffer *buf;
    buf = malloc(sizeof(buffer));
    init_buffer(buf);

    pthread_t t[thread_count];
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_mutex_lock(&buf->in->mut);
    for (int i = 0; i < thread_count; i++) {
	pthread_create(&t[i], &attr, mandel_consumer, buf);
    }

    for (int i = 0; i < w; i++) {
	for (int j = 0; j < h; j++) {
	    sem_wait(&buf->full);
	    pthread_mutex_lock(&buf->mut);


	    buf->pix[buf->pos] = malloc(sizeof(mandel_t));
	    buf->pix[buf->pos]->x = i;
	    buf->pix[buf->pos]->y = j;
	    buf->pix[buf->pos]->bail_out = bail_out;
	    buf->pix[buf->pos]->max_iter = max_iter;
	    buf->pix[buf->pos]->field.arr = field;
	    buf->pix[buf->pos]->field.h = h;
	    buf->pix[buf->pos]->field.w = w;
	    buf->pos += 1;

	    sem_post(&buf->empty);
	    pthread_mutex_unlock(&buf->mut);
	}
    }
    pthread_mutex_unlock(&buf->in->mut);
    printf("Finished filling the buffer");

    for (int i = 0; i < thread_count; i++) {
	pthread_join(t[i], NULL);
    }

    free(buf->in);
    free(buf->pix);
    free(buf);
}

int main(int argc, char **argv)
{

    int *pix_field;

    pix_field = malloc(WIDTH * HEIGHT * sizeof(int));

    printf
	("WIDTH: %d\nHEIGHT %d\nBAIL OUT %d\nMAX ITER %d\nTHREAD COUNT %d",
	 WIDTH, HEIGHT, BAIL_OUT, MAX_ITER, THREAD_COUNT);


    mandelbrot(pix_field, WIDTH, HEIGHT, BAIL_OUT, MAX_ITER, THREAD_COUNT);

    pix_row rows[WIDTH];
    pix p;

    char filename[30];
    image img;
    p.r = 200;
    p.g = 200;
    p.b = 200;

    for (int i = 0; i < WIDTH; i++) {
	rows[i].p = malloc(HEIGHT * sizeof(pix));
	for (int j = 0; j < HEIGHT; j++) {
	    p.r = map(pix_field[i * HEIGHT + j], 0, MAX_ITER, 0, 255);
	    p.g = map(pix_field[i * HEIGHT + j], 0, MAX_ITER, 0, 255);
	    p.b = map(pix_field[i * HEIGHT + j], 0, MAX_ITER, 0, 255);
	    rows[i].p[j] = p;
	}
    }
    sprintf(filename, "mandel.png");
    img = initialize_png("mandelbrot", filename, WIDTH, HEIGHT);
    write_image(&img, rows);
    finish_image(&img);
    for (int i = 0; i < WIDTH; i++) {

	free(rows[i].p);
    }

    free(pix_field);

    return (0);
}
