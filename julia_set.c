#include <stdio.h>

typedef unsigned char uchar;

struct colour { uchar r, g, b; };

const int         channel_size = 256;
const int         num_colours = 256 * 256 * 256;
const int         width = 4096, height = 4096;

int               julia                   [width][height];

int quantise (double v) {
  if (v >= 1.0) v = 0.0;
  if (v < 0.0) v = 0.0;
  return (int) (v * (double) channel_size);
}

int main () {
  { // Generate Julia Set ----------------------------------
    const int iteration_limit = 196;
    const double zoom = 1.0;
    const double x_pos = 0.0, y_pos = 0.0;
    const double real = -0.7, imaginary = 0.27015;
    for (int x = width; x--;) {
      for (int y = height; y--;) {
        double rp = real, ip = imaginary;
        int z = 0;
        double rn = 1.5 * (x - width / 2.0) / (0.5 * zoom * width) + x_pos;
        double in = (y - height / 2.0) / (0.5 * zoom * height) + y_pos;
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
    printf("P6 %i %i 255 ", width, height);
    for (int i = 0; i < num_colours; i++) {
      int x = i % width, y = (i - x) / height;
      int v = julia [x][y];
      printf ("%c%c%c", v, v, v);
    }
  }

  return 0;
}
