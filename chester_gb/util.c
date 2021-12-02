
#include<stddef.h>
#include <stdbool.h>
#include <stddef.h>
#include "util.h"

void memset(void *dest, int val, size_t len) {

  unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;
}

void memcpy(const void *dest, const void *src, size_t len) {
    if (dest < src)
    {
      const char *firsts = (const char *) src;
      char *firstd = (char *) dest;
      while (len--)
	*firstd++ = *firsts++;
    }
  else
    {
      const char *lasts = (const char *)src + (len-1);
      char *lastd = (char *)dest + (len-1);
      while (len--)
        *lastd-- = *lasts--;
    }
}
