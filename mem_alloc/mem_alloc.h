#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <stdlib.h>
#include <inttypes.h>

/**
 * Fills the pointer's array of the given size with each element points
 * to the next element.
 */
void fill_memory_seq(uint64_t *memory, size_t size);

/**
 * Fills the pointer's array of the given size with each element
 * pointing to another pseudo-random element in the array. All the
 * elements of the array are pointed to exactly by one other
 * element. The last element always points to the first one.
 */
void fill_memory_rand(uint64_t *memory, size_t size);

#endif
