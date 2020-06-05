#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ENABLE_SHUFFLE 1
#define USE_MORTON_CODES 1

#define CHANNEL_SIZE 256
#define NUM_COLOURS 256 * 256 * 256
#define WIDTH 4096
#define HEIGHT 4096

typedef unsigned int uint;
typedef unsigned char uchar;

struct colour { uchar r, g, b; };

int               julia                   [WIDTH][HEIGHT];
struct colour     base_allrgb_sequence    [NUM_COLOURS];
int               histogram               [CHANNEL_SIZE];
int               shuffle_lookup          [NUM_COLOURS];
struct colour     output                  [WIDTH][HEIGHT];

uint morton_code (struct colour c) {
  uint code = 0;

  for (int i = 0; i < 8; ++i) {
    // create a mask for the ith bit
    uint mask = ((uint)1 << i);
    // select just the ith bit
    uint r = c.r & mask;
    uint g = c.g & mask;
    uint b = c.b & mask;
    // shift the ith bit to the lsb position
    uint r1 = ((uint) r) >> i;
    uint g1 = ((uint) g) >> i;
    uint b1 = ((uint) b) >> i;
    // shift the lsb to it's target postion
    uint r2 = r1 << (0 + i * 3);
    uint g2 = g1 << (1 + i * 3);
    uint b2 = b1 << (2 + i * 3);
    // combine
    code |= r2 | g2 | b2;
  }
  assert (code < NUM_COLOURS);
  return code;
}

struct colour inv_morton_code (uint code) {
  uint r = 0;
  uint g = 0;
  uint b = 0;
  for (int i = 0; i < 8; ++i) {
    // create a mask for the target bit
    uint rmask = ((uint)1 << (0 + i * 3));
    uint gmask = ((uint)1 << (1 + i * 3));
    uint bmask = ((uint)1 << (2 + i * 3));
    // select just the target bit
    uint r1 = code & rmask;
    uint g1 = code & gmask;
    uint b1 = code & bmask;
    // shift the target bit to the lsb position
    uint r2 = ((uint) r1) >> (0 + i * 3);
    uint g2 = ((uint) g1) >> (1 + i * 3);
    uint b2 = ((uint) b1) >> (2 + i * 3);
    // shift the lsb to it's desired postion
    uint r3 = r2 << i;
    uint g3 = g2 << i;
    uint b3 = b2 << i;
    // combine
    r |= r3;
    g |= g3;
    b |= b3;
  }
  assert (code < NUM_COLOURS);
  struct colour c;
  c.r = (uchar) r;
  c.g = (uchar) g;
  c.b = (uchar) b;
  return c;
}

void shuffle (int* array, int n) {
  if (n > 1) {
    int i;
    for (i = 0; i < n - 1; i++) {
      int j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

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
#if USE_MORTON_CODES
    for (int i = 0; i < NUM_COLOURS; ++i) {
      base_allrgb_sequence[i] = inv_morton_code(i);
    }
#else
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
#endif
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
      shuffle_lookup [i] = i;
    }
#if ENABLE_SHUFFLE
    for (int i = 0; i < CHANNEL_SIZE; ++i) {
      int start = 0;
      if (i > 0) start = cumulative_histogram [i-1];
      int range = cumulative_histogram [i] - start;
      shuffle(&shuffle_lookup [start], range);
    }
#endif
    for (int i = 0; i < NUM_COLOURS; i++) {
      int x = i % WIDTH, y = (i - x) / HEIGHT;
      uchar j = julia [x][y];
      int uniqueId = --cumulative_histogram [j];
      int index = shuffle_lookup [uniqueId];
      output [x][y] = base_allrgb_sequence [index];
    }
  }

  { // Output to PPM file. ---------------------------------
    printf("P6 %i %i 255 ", WIDTH, HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
      int x = i % WIDTH, y = (i - x) / HEIGHT;
      struct colour v = output [x][y];
      printf ("%c%c%c", v.r, v.g, v.b);
    }
  }
  return 0;
}
