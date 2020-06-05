#include <stdio.h>

#define CHANNEL_SIZE 256
#define NUM_COLOURS 256 * 256 * 256
#define WIDTH 4096
#define HEIGHT 4096

typedef unsigned char uchar;

struct colour { uchar r, g, b; };

int julia [WIDTH][HEIGHT];

int quantise (double v) {
  if (v >= 1.0) v = 0.0;
  if (v < 0.0) v = 0.0;
  return (int) (v * (double) CHANNEL_SIZE);
}

int main () {
  { // Generate Julia Set ----------------------------------
    const int iteration_limit = 196;
    const double zoom = 1.0;
    const double x_pos = 0.0, y_pos = 0.0;
    const double real = -0.7, imaginary = 0.27015;
    for (int x = WIDTH; x--;) {
      for (int y = HEIGHT; y--;) {
        double rp = real, ip = imaginary;
        int z = 0;
        double rn = 1.5 * (x - WIDTH / 2.0) / (0.5 * zoom * WIDTH) + x_pos;
        double in = (y - HEIGHT / 2.0) / (0.5 * zoom * HEIGHT) + y_pos;
        for(z = 0; z < iteration_limit; z++) {
          rp = rn, ip = in;
          rn = rp * rp - ip * ip + real;
          in = 2.0 * rp * ip + imaginary;
          if((rn * rn + in * in) > 4.0)
            break;
        }
        julia [x][y] = quantise ((double) z / (double) iteration_limit);
      }
    }
  }

  { // Output to PPM file. ---------------------------------
    printf("P6 %i %i 255 ", WIDTH, HEIGHT);
    for (int i = 0; i < NUM_COLOURS; i++) {
      int x = i % WIDTH, y = (i - x) / HEIGHT;
      int v = julia [x][y];
      printf ("%c%c%c", v, v, v);
    }
  }

  return 0;
}
