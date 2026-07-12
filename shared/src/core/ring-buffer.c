#include "core/ring-buffer.h"

/* A fixed-size FIFO (circular) buffer. Bytes are pushed at write_index and popped
   at read_index; each index wraps around with a bitmask, which requires the capacity
   to be a power of two. One slot is deliberately left unused so "empty" (read==write)
   and "full" can be told apart. The writer (USART2 ISR) and the reader (main loop)
   each advance only their own index, so on a single-core MCU no locking is needed. */

void ring_buffer_setup(ring_buffer_t* rb, uint8_t* buffer, uint32_t size){
    rb->buffer = buffer;
    rb->read_index = 0;
    rb->write_index = 0;
    rb->mask = size-1;          // size MUST be a power of two so (index & mask) wraps
}

// Empty when the reader has caught up to the writer.
bool ring_buffer_empty(ring_buffer_t* rb){
    return rb->read_index == rb->write_index;
}

// Pop one byte into *byte. Returns false (nothing changed) if the buffer is empty.
bool ring_buffer_read(ring_buffer_t* rb, uint8_t* byte){
    // Snapshot both indices once so the value can't shift mid-function if the ISR runs.
    uint32_t local_read_index = rb->read_index;
    uint32_t local_write_index = rb->write_index;

    if (local_read_index == local_write_index) {
        return false;                                   // empty: nothing to read
    }

    *byte = rb->buffer[local_read_index];
    local_read_index = (local_read_index + 1) & rb->mask;   // advance, then wrap
    rb->read_index = local_read_index;                  // publish new position last

    return true;
}

// Push one byte. Returns false (byte dropped) if the buffer is full.
bool ring_buffer_write(ring_buffer_t* rb, uint8_t byte){
    uint32_t local_write_index = rb->write_index;
    uint32_t local_read_index = rb->read_index;

    uint32_t next_write_index = (local_write_index + 1) & rb->mask;

    if(next_write_index==local_read_index){
        return false;                                   // full: would collide with reader
    }

    rb->buffer[local_write_index] = byte;
    rb->write_index = next_write_index;                 // publish only after the byte is stored
    return true;
}

