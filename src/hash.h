#ifndef SRC_HASH_H
#define SRC_HASH_H

#include <stdint.h>
#include <stddef.h>

uint32_t hash_fnv1a_32(const char *data, size_t len);

#endif

