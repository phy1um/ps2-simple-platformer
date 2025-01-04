#include "hash.h"
#include <stdint.h>
#include <stddef.h>

uint32_t hash_fnv1a_32(const char *data, size_t len) {
  uint32_t hash = 2166136261;
  size_t i = 0;
  while(data[i] != 0) {
    hash ^= data[i];
    hash *= 16777619;
    i += 1;
  }
  return hash;
}

