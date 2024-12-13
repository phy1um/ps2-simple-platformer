#include "hash.h"
#include <stdint.h>
#include <stddef.h>

uint32_t hash_fnv1a_32(const char *data, size_t len) {
  uint32_t hash = 2166136261;
  for (size_t i = 0; i < len; i++) {
    hash ^= data[i];
    hash *= 16777619;
  }
  return hash;
}

