#include <stdio.h>

#define CHANNEL_SIZE 256
#define NUM_COLOURS 256 * 256 * 256
#define WIDTH 4096
#define HEIGHT 4096

typedef unsigned char uchar;

struct colour { uchar r, g, b; };

int               julia                   [WIDTH][HEIGHT];
struct colour     base_allrgb_sequence    [NUM_COLOURS];
int               histogram               [CHANNEL_SIZE];
struct colour     output                  [WIDTH][HEIGHT];

int quantise (double v) {
  if (v >= 1.0) v = 0.0;
  if (v < 0.0) v = 0.0;
  return (int) (v * (double) CHANNEL_SIZE);
}

int main () {
  { // Generate Julia Set. ---------------------------------
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

  { // Generate one dimensional RGB sequence. --------------
    int counter = 0;
    for (int r = 256; r--;) {
      for (int g = 256; g--;) {
        for (int b = 256; b--;) {
          base_allrgb_sequence[counter].r = r;
          base_allrgb_sequence[counter].g = g;
          base_allrgb_sequence[counter].b = b;
          ++counter;
        }
      }
    }
  }

  { // Generate histogram. ---------------------------------
    for (int i = CHANNEL_SIZE; i--;) {
      histogram[i] = 0;
    }
    for (int i = 0; i < NUM_COLOURS; i++) {
      int x = i % WIDTH, y = (i - x) / HEIGHT;
      uchar j = julia [x][y];
      histogram [j]++;
    }
  }

  { // Create output data. ---------------------------------
    int cumulative_histogram[CHANNEL_SIZE];
    for (int i = 0; i < CHANNEL_SIZE; i++) {
      cumulative_histogram [i] = histogram[i];
      if (i > 0) cumulative_histogram [i] += cumulative_histogram [i-1];
    }
    for (int i = 0; i < NUM_COLOURS; i++) {
      int x = i % WIDTH, y = (i - x) / HEIGHT;
      uchar j = julia [x][y];
      int uniqueId = --cumulative_histogram [j];
      output [x][y] = base_allrgb_sequence [uniqueId];
    }
  }

  { // Output to PPM file. ---------------------------------
    printf("P6 %i %i 255 ", WIDTH, HEIGHT);
    for (int i = 0; i < NUM_COLOURS; i++) {
      int x = i % WIDTH, y = (i - x) / HEIGHT;
      struct colour v = output [x][y];
      printf ("%c%c%c", v.r, v.g, v.b);
    }
  }

  return 0;
}
