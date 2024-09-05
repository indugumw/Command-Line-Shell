#include <stdlib.h>
#include <stdio.h>

static const char* test1 = "𓀬";

unsigned utf8_char_size(unsigned char val) {
    if (val < 128) {
        return 1;
    } else if (val < 224) {
        return 2;
    } else if (val < 240) {
        return 3;
    } else {
        return 4;
    }
}

unsigned utf8_count_chars(const unsigned char* data)
{
  unsigned total = 0;
  while(*data != 0) {
    unsigned char_width = utf8_char_size(*data);
    total++;
    data += char_width;
  }
  return total;
}

int main(void) {
  fprintf(stdout, "The count is %u\n", utf8_count_chars((unsigned char*)test1));
  return 0;
}