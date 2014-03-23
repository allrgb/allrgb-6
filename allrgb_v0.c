#include <stdio.h>

int main() {
  printf("P6 4096 4096 255 ");
  for (int r = 256; r--;)
    for (int g = 256; g--;)
      for (int b = 256; b--;)
        printf("%c%c%c", r, g, b);
}
