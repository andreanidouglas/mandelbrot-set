#include "mandelbrot.h"

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

void mandelbrot(int *field, int w, int h, int bail_out, int max_iter,
		int thread_count)
{

	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			double x = map(i, 0, w, -1, 1);
			double y = map(j, 0, h, -1, 1);

			int k = mandelbrot_pix(x, y, BAIL_OUT, MAX_ITER);
			fprintf(stdout, "\nComputing %d %d = %d", i, j, k);
			field[i + j * h] = k;
		}
	}

}

void *produce(void *b)
{
	mandel_buf_t *buf;
	mandel_t *m;

	double x;
	double y;

	buf = (mandel_buf_t *) b;

	for (int i = 0; i < buf->prop.width; i++) {
		for (int j = 0; j < buf->prop.height; j++) {

			m = malloc(sizeof(mandel_t));
			m->x = i;
			m->y = j;
			enqueue(buf->q, m, buf->lock);
		}
	}
	buf->produce_end = 1;
	return NULL;
}

void *consume(void *b)
{
	mandel_buf_t *buf;
	mandel_t *m;
	double x, y;
	int k, i;

	buf = (mandel_buf_t *) b;


	/* This will not work on a multi-consumer program 
	while (i < buf->prop.width * buf->prop.height) { */
	for(;;){
		if (!is_empty(buf->q)) {
			m = dequeue(buf->q, buf->lock);
			x = map(m->x, 0, buf->prop.width, -1, 1);
			y = map(m->y, 0, buf->prop.height, -1, 1);
			//fprintf(stdout, "\nSending %d %d", m->x, m->y, k);

			k = mandelbrot_pix(x, y, buf->prop.bail_out,
					buf->prop.max_iter);
			buf->result_field[m->y + m->x * buf->prop.height] = k;
			free(m);
			i++;
		}
		if(is_empty(buf->q) && buf->produce_end){
			return NULL;
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{

	pthread_t consumer1, consumer2;
	pthread_t producer;
	pthread_attr_t attr;
	pthread_mutex_t lock;

	printf
	    ("WIDTH: %d\nHEIGHT %d\nBAIL OUT %d\nMAX ITER %d\nTHREAD COUNT %d",
	     WIDTH, HEIGHT, BAIL_OUT, MAX_ITER, THREAD_COUNT);

	pthread_attr_init(&attr);

	/* Define current mandelbrot set properties */
	mandel_prop_t prop;
	prop.bail_out = BAIL_OUT;
	prop.max_iter = MAX_ITER;
	prop.height = HEIGHT;
	prop.width = WIDTH;

	/* Initialize the buffer for threads to work with */
	mandel_buf_t buf;
	buf.prop = prop;
	queue_t *queue;
	queue = initialize_queue();
	pthread_mutex_init(&lock, NULL);
	buf.lock = &lock;
	buf.produce_end = 0;
	buf.result_field = malloc(sizeof(int) * WIDTH * HEIGHT);

	buf.q = queue;

	/* Create and start threads */
	pthread_create(&producer, &attr, produce, &buf);
	pthread_create(&consumer1, &attr, consume, &buf);
	pthread_create(&consumer2, &attr, consume, &buf);



	pthread_join(producer, NULL);
	printf("\nFinished producer!");
	pthread_join(consumer1, NULL);
	printf("\nFinished consumer 1!");
	pthread_join(consumer2, NULL);
	printf("\nFinished consumer 2!");

	/* Create the png image with the result buffer */
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
			p.r =
			    map(buf.result_field[i * HEIGHT + j], 0,
				MAX_ITER, 0, 255);
			p.g =
			    map(buf.result_field[i * HEIGHT + j], 0,
				MAX_ITER, 0, 255);
			p.b =
			    map(buf.result_field[i * HEIGHT + j], 0,
				MAX_ITER, 0, 255);
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

	free(buf.result_field);

	return (0);
}
