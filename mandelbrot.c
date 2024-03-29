#include "mandelbrot.h"
#include "raylib.h"

double map(int value, int min, int max, int map_min, int map_max) {
  double R = (double)(map_max - map_min) / (double)(max - min);
  double y = map_min + (value * R) + R;
  return y;
}

int mandelbrot_pix(double x, double y, int bail_out, int max_iter) {
  int k;
  double real, imaginary, x2, y2;

  x += -1;

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
    y = imaginary + y2;

    if ((x * x + y * y) > bail_out) {
      return k;
    }
  }
  return max_iter;
}

void *produce(void *b) {
  mandel_buf_t *buf;
  mandel_t m;

  buf = (mandel_buf_t *)b;

  pthread_mutex_lock(buf->lock);
  for (int i = 0; i < buf->prop.width; i++) {
    for (int j = 0; j < buf->prop.height; j++) {
      m.x = i;
      m.y = j;
      enqueue(buf->q, m, buf->lock);
    }
  }
  buf->produce_end = 1;

  pthread_mutex_unlock(buf->lock);
  return NULL;
}

void reset_buffer(mandel_t *buf) {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buf[i].x = -1;
    buf[i].y = -1;
  }
}

int fill_buffer(mandel_t *buf, queue_t *q, pthread_mutex_t *lock) {
  int size = 0;

  pthread_mutex_lock(lock);
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buf[i] = dequeue(q, lock);
    if (buf[i].x != -1 && buf[i].y != -1)
      size++;
  }
  
  pthread_mutex_unlock(lock);
  return size;
}

void *consume(void *b) {
  mandel_buf_t *buf;
  mandel_t m;
  double x, y;
  int k;
  ssize_t thread_id;
  mandel_t mandel_buf[BUFFER_SIZE];

  buf = (mandel_buf_t *)b;
  thread_id = buf->thread_num;
  pthread_mutex_unlock(buf->lock);

  for (;;) {

    int size = fill_buffer(mandel_buf, buf->q, buf->lock);
    if (size == 0) {
      printf("Thread finished %zd. Joining main\n", thread_id);
      fflush(stdout);
      return NULL;
    }
    

    //printf("Buffer for %zd filled\n", thread_id);
    
    buf->thread_pool[thread_id]++;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      m = mandel_buf[i];
      if (m.x == -1 && m.y == -1)
        break;
      x = map(m.x, 0, buf->prop.width, -1.0, 1.0);
      y = map(m.y, 0, buf->prop.height, -1.0, 1.0);

      k = mandelbrot_pix(x, y, buf->prop.bail_out, buf->prop.max_iter);
      buf->result_field[m.y + m.x * buf->prop.height] = k;
    }

    reset_buffer(mandel_buf);
  }
  

  return NULL;
}

int main() {

  pthread_t consumer[THREAD_COUNT];
  // pthread_t producer;
  pthread_attr_t attr;
  pthread_mutex_t lock;

  printf("WIDTH: %d\nHEIGHT %d\nBAIL OUT %d\nMAX ITER %d\nTHREAD COUNT %d\n",
         WIDTH, HEIGHT, BAIL_OUT, MAX_ITER, THREAD_COUNT);
  fflush(stdout);
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
  /*
      // Create and start threads
      pthread_create(&producer, &attr, produce, &buf);

      pthread_join(producer, NULL);
  */

  produce(&buf);

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_mutex_lock(buf.lock);
    buf.thread_num = i;      
    pthread_create(&consumer[i], &attr, consume, &buf);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(consumer[i], NULL);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
          printf("Thread [%d]: %zd {%zd}\n\n", i, buf.thread_pool[i], buf.thread_pool[i] * BUFFER_SIZE);
  }
  destroy_queue(buf.q);

  const int window_width = WIDTH;
  const int window_height = HEIGHT;
  InitWindow(window_width, window_height, "Mandelbrot set");
  SetTargetFPS(60);

  Image mandelbrot_image = GenImageColor(WIDTH, HEIGHT, RED);
  for (int i = 0; i < HEIGHT; i++) {
      for (int j=0; j<WIDTH; j++) {
        if (buf.result_field[i * HEIGHT + j] == MAX_ITER)
            ImageDrawPixel(&mandelbrot_image, j, i, BLACK);
        else 
            ImageDrawPixel(&mandelbrot_image, j, i, RED);
      }
  }


  ImageRotate(&mandelbrot_image, -90);
  Texture2D canvas = LoadTextureFromImage(mandelbrot_image);

  while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(GRAY);
      DrawTexture(canvas, 0, 0, WHITE);
      EndDrawing();
  }

  CloseWindow();

   
  free(buf.result_field);

  return (0);
}
