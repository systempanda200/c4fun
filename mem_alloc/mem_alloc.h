#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <stdlib.h>
#include <inttypes.h>

enum access_mode_t {
  access_seq,
  access_rand
};

/**
 * Fills the given memory region either sequentially or pseudo
 * randomly.
 *
 * If sequential, fills the pointer's array of the given size with
 * each element points to the next element.
 *
 * If random, fills the pointer's array of the given size with each
 * element pointing to another pseudo-random element in the array. All
 * the elements of the array are pointed to exactly by one other
 * element. The last element always points to the first one.
 */
void fill_memory(uint64_t *memory, size_t size, enum access_mode_t access_mode);

#endif
