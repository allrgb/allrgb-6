# 256 x 256 x 256

[allrgb.com](http://www.allrgb.com) is home to an interesting programming challenge: "Create images with one pixel for every RGB colour (16777216 total); not one colour missing, and not one colour used more than once."

## A minimal solution

It's easy to solve the challenge in a few lines of `C` using the `NetPBM` format for the output image:

```c
#include <stdio.h>

int main() {
  printf("P6 4096 4096 255 ");
  for (int r = 256; r--;)
    for (int g = 256; g--;)
      for (int b = 256; b--;)
        printf("%c%c%c", r, g, b);
}

```

To put the code to the test put it in a file called [allrgb_v0.c](/allrgb_v0.c) and run:

```bash
gcc allrgb_v0.c -o allrgb_v0
./allrgb_v0 >> allrgb_v0.ppm
```

It will generate something like this:

[![](/docs/sm_allrgb_minimal.png "A minimal solution.")](/docs/allrgb_minimal.png)

## Generating some variety

To create a more interesting solution the subsequent examples on this page are based on a couple of simple principles:

* Step 1: Generate a grayscale image (8 bits per pixel) to provide a base for varied content.
* Step 2: Colourise the grayscale image by distributing each unique colour value across the image. 

### The Julia Set

The Julia Set fractal can be generated with a small amount of code and is well suited to producing a grayscale image - each input point outputs a single real number between 0.0 and 1.0 which can be quantized to produce a grayscale image:

```c
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
  { // Generate Julia Set. ---------------------------------
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
```

Put the code in a file called [julia_set.c](/julia_set.c) and run:

```bash
gcc julia_set.c -o julia_set
./julia_set >> julia_set.ppm
```

It will generate something like this:

[![](/docs/sm_julia_set.png "Black & White Julia Set.")](/docs/julia_set.png)

### Histogram colouring method

Take the grayscale image above, each pixel is one of 256 possible values, then generate a histogram to enumerate the occurances of each grayscale value in the image.  This simple grouping can then be used as the basis for a colouring method.

```c
int histogram[channel_size];

{ // Generate histogram. ---------------------------------


  for (int i = channel_size; i--;) {
    histogram[i] = 0;
  }
  for (int i = 0; i < num_colours; i++) {
    int x = i % width, y = (i - x) / height;
    uchar j = julia [x][y];
    histogram [j]++;
  }
}
```

Given all possible RGB values (256x256x256) organised into a one dimensional list `base_allrgb_sequence`, assign sequentially from that list subsets of those values to each bar of the histogram - the cardinality of the subset being equal to the size of the bar.

Next iterate over the grayscale image, for each grayscale value: remove an RGB value from the associated subset using it to then replace the original grayscale value.  The result being an RGB image.

```c
struct colour base_allrgb_sequence[num_colours];
struct colour output[width][height];

{ // Create output data. ---------------------------------
  int cumulative_histogram[channel_size];
  for (int i = 0; i < channel_size; i++) {
    cumulative_histogram [i] = histogram[i];
    if (i > 0) cumulative_histogram [i] += cumulative_histogram [i-1];
  }
  for (int i = 0; i < num_colours; i++) {
    int x = i % width, y = (i - x) / height;
    uchar j = julia [x][y];
    int uniqueId = --cumulative_histogram [j];
    output [x][y] = base_allrgb_sequence [uniqueId];
  }
}
```

### Solutions in this demo

This demo creates four image variants by providing varied sortings of the one dimensional input data (`base_allrgb_sequence`) required by the histogram colouring method.

#### Variant 1 - Sequential RGB Subsets

This variant creates a one dimensional sequence based upon the simple approach used in the first example on this page:

```c
typedef unsigned char uchar;

struct colour { uchar r, g, b; };

colour base_allrgb_sequence [num_colours];

void generate_base_allrgb_sequence () {
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
```

Find the code here [allrgb_v1.c](/allrgb_v1.c) and run:

```bash
gcc allrgb_v1.c -o allrgb_v1
./allrgb_v1 >> allrgb_v1.ppm
```

It will produce:

[![](/docs/sm_allrgb_group.png)](/docs/allrgb_group.png)

The start of the one dimensional input sequence contains more red and the end contains more blue, this means that lower quantized values in our fractal's output sequence will be associated with RGB subsets that contain more red and higher values associated with RGB subsets that contain more blue.

#### Variant 2 - Shuffled RGB Subsets

To reduce the sequential noise in variant 1, this variant shuffles each subset of RGB values:

The following code implements the colouring algorithm and has been used to test both sequential and random iteration through the subsets:

```c
int shuffle_lookup[num_colours];

{ // Create output data. ---------------------------------
  int cumulative_histogram[channel_size];
  for (int i = 0; i < channel_size; i++) {
    cumulative_histogram [i] = histogram[i];
    if (i > 0) cumulative_histogram [i] += cumulative_histogram [i-1];
  }
  for (int i = 0; i < num_colours; i++) {
    shuffle_lookup [i] = i;
  }
  for (int i = 0; i < channel_size; ++i) {
    int start = 0;
    if (i > 0) start = cumulative_histogram [i-1];
    int range = cumulative_histogram [i] - start;
    shuffle(&shuffle_lookup [start], range);
  }
  for (int i = 0; i < num_colours; i++) {
    int x = i % width, y = (i - x) / height;
    uchar j = julia [x][y];
    int uniqueId = --cumulative_histogram [j];
    int index = shuffle_lookup [uniqueId];
    output [x][y] = base_allrgb_sequence [index];
  }
}
```

Find the code here [allrgb_v2.c](/allrgb_v2.c) and run:

```bash
gcc allrgb_v2.c -o allrgb_v2
./allrgb_v2 >> allrgb_v2.ppm
```

It will produce:

[![](/docs/sm_allrgb_group_shuffle.png)](/docs/allrgb_group_shuffle.png)

The boundaries between small steps of grayscale values in the original Julia Set image are here clearly visible as the average colour of each subset of RGB values associated with each grayscale value will not incremently smoothly.

#### Variant 3 - Morton Codes

Morton Codes are a method for interleaving multi-dimensional sequences into a single-dimension.  The following diagram illustrates how two dimensional co-ordinates can be interwoven into a one-dimensional sequence:

![](/docs/zcurve.png)

Morton Codes are also know as Z-Curves - not surprising given the shapes seen above.  The process can be easily applied to any number of dimensions.

This variant implements Morton Coding for three-dimensional colour data (representing red, green and blue colour channels), it then sequentially traverses the implemented Morton Code set to produce an interesting one dimensional sequence of all possible RGB values.  The implementation details involve converting the decimal colour values to binary and then interleaving the bits of each colour channel:

<ul>
  <li>
    (<font color="red">R</font>, <font color="green">G</font>, <font color="blue">B</font>): (<font color="red">255</font>, <font color="green">89</font>, <font color="blue">144</font>) => (<font color="red">11111111</font>, <font color="green">10010000</font>, <font color="blue">01011001</font>)
  </li>
  <li>
    Interleaving the bits results in: <font color="red">1</font><font color="green">1</font><font color="blue">0</font><font color="red">1</font><font color="green">0</font><font color="blue">1</font><font color="red">1</font><font color="green">0</font><font color="blue">0</font><font color="red">1</font><font color="green">1</font><font color="blue">1</font><font color="red">1</font><font color="green">0</font><font color="blue">1</font><font color="red">1</font><font color="green">0</font><font color="blue">0</font><font color="red">1</font><font color="green">0</font><font color="blue">0</font><font color="red">1</font><font color="green">0</font><font color="blue">1</font>
  </li>
  <li>
    Padded with zeros to a 32-bit integer: 00000000<font color="red">1</font><font color="green">1</font><font color="blue">0</font><font color="red">1</font><font color="green">0</font><font color="blue">1</font><font color="red">1</font><font color="green">0</font><font color="blue">0</font><font color="red">1</font><font color="green">1</font><font color="blue">1</font><font color="red">1</font><font color="green">0</font><font color="blue">1</font><font color="red">1</font><font color="green">0</font><font color="blue">0</font><font color="red">1</font><font color="green">0</font><font color="blue">0</font><font color="red">1</font><font color="green">0</font><font color="blue">1</font> => 14,056,229th Morton Code
  </li>
</ul>

The following functions implement conversion of RGB colour values to and from a Morton Codes:

```c
typedef unsigned int uint;
typedef unsigned char uchar;

struct colour { uchar r, g, b; };

uint morton_code (colour c) {
  uint code = 0;

  for (int i = 0; i < 8; ++i) {
    // create a mask for the ith bit
    uint mask = ((uint)1 << i);
    // select just the ith bit
    uint r = c.r &amp; mask;
    uint g = c.g &amp; mask;
    uint b = c.b &amp; mask;
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
  assert (code < NUM_PIXELS);
  return code;
}

colour inv_morton_code (uint code) {
  uint r = 0;
  uint g = 0;
  uint b = 0;
  for (int i = 0; i < 8; ++i) {
    // create a mask for the target bit
    uint rmask = ((uint)1 << (0 + i * 3));
    uint gmask = ((uint)1 << (1 + i * 3));
    uint bmask = ((uint)1 << (2 + i * 3));
    // select just the target bit
    uint r1 = code &amp; rmask;
    uint g1 = code &amp; gmask;
    uint b1 = code &amp; bmask;
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
  assert (code < NUM_PIXELS);
  colour c;
  c.r = (uchar) r;
  c.g = (uchar) g;
  c.b = (uchar) b;
  return c;
}
```

The colour distribution of iterating through Morton codes 0 - 16777215 can be seen here:

![](/docs/morton_1d.png)

Find the adjusted code here [allrgb_v3.c](/allrgb_v3.c) and run:

```bash
gcc allrgb_v3.c -o allrgb_v3
./allrgb_v3 >> allrgb_v3.ppm
```

With shuffling disabled it will produce:

[![](/docs/sm_allrgb_morton.png)](/docs/allrgb_morton.png)

and with shuffling enabled:

[![](/docs/sm_allrgb_morton_shuffle.png)](/docs/allrgb_morton_shuffle.png)

It's interesting to look at the blue/yellow step of the unshuffled image, it's clear from which portion of the one-dimensional base sequence it was built:

![](/docs/morton_1d.png)


[license]: https://raw.githubusercontent.com/sungiant/allrgb/master/LICENSE


