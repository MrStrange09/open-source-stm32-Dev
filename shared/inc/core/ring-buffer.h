#ifndef INC_RING_BUFFER_H
#define INC_RING_BUFFER_H

#include "common-defines.h"

typedef struct ring_buffer_t {
    uint8_t* buffer;        // caller-provided storage (its length must be a power of two)
    uint32_t mask;          // capacity - 1; ANDing an index with this wraps it
    uint32_t read_index;    // next slot to read from  (advanced only by the reader)
    uint32_t write_index;   // next slot to write to   (advanced only by the writer)
} ring_buffer_t;

void ring_buffer_setup(ring_buffer_t* rb, uint8_t* buffer, uint32_t size);
bool ring_buffer_empty(ring_buffer_t* rb);
bool ring_buffer_write(ring_buffer_t* rb, uint8_t byte);
bool ring_buffer_read(ring_buffer_t* rb, uint8_t* byte);

#endif // INC_RING_BUFFER_H  