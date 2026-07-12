# Ring Buffer (UART RX)

This checkpoint replaces the one-byte RX "mailbox" from the UART 1B driver with a
proper **ring buffer** (circular FIFO), so bytes arriving faster than the main loop
reads them are queued instead of dropped.

See `uart-driver-1b.md` for the driver this builds on.

## What A Ring Buffer Is

A ring buffer is a fixed array with two moving cursors:

- `write_index`: where the **producer** (the USART2 ISR) puts the next byte.
- `read_index`: where the **consumer** (the main loop) takes the next byte.

Both cursors only ever move forward and **wrap** back to `0` when they reach the end,
so the array is used like a circle. No data is ever shifted; only the two indices move.

```text
     read_index                write_index
        |                           |
        v                           v
  [ _ ][a][b][c][d][ _ ][ _ ][ _ ] ...
        \_______ unread ______/
```

## Files

```text
shared/src/core/ring-buffer.c
shared/inc/core/ring-buffer.h
```

The struct is deliberately tiny:

```c
typedef struct ring_buffer_t {
    uint8_t* buffer;        // caller-provided storage (length must be a power of two)
    uint32_t mask;          // capacity - 1
    uint32_t read_index;
    uint32_t write_index;
} ring_buffer_t;
```

## The Power-Of-Two / Mask Trick

`ring_buffer_setup()` stores `mask = size - 1` instead of the size itself. Because the
capacity is a power of two (here `128`), advancing an index becomes a cheap AND:

```c
index = (index + 1) & rb->mask;   // wraps 127 -> 0 with no branch, no modulo
```

`& mask` keeps only the low bits, which is exactly the wrap-around that `% size` would
give, but without a division. **This only works if `size` is a power of two** — that is
the one rule to remember when picking `RING_BUFFER_SIZE`.

## Empty vs Full (the sacrificed slot)

Both "empty" and "full" want to make the two indices equal, so they'd be
indistinguishable. The fix: **never let the buffer completely fill** — one slot is
always left open.

- **Empty:** `read_index == write_index`.
- **Full:** the *next* write position would land on `read_index`, so the write is
  refused.

```c
bool ring_buffer_empty(ring_buffer_t* rb){
    return rb->read_index == rb->write_index;
}

bool ring_buffer_write(ring_buffer_t* rb, uint8_t byte){
    uint32_t next_write_index = (rb->write_index + 1) & rb->mask;
    if (next_write_index == rb->read_index) {
        return false;                 // full -> byte dropped
    }
    rb->buffer[rb->write_index] = byte;
    rb->write_index = next_write_index;
    return true;
}
```

So a 128-slot buffer holds at most 127 queued bytes. That is the price of telling empty
and full apart with just two indices.

## Why It Is Safe Without Locks

The producer (ISR) touches only `write_index`; the consumer (main loop) touches only
`read_index`. Each side reads the other's index but never writes it. On a single-core
Cortex-M this is enough to avoid corruption without disabling interrupts, as long as
each function **stores the byte first and publishes the moved index last** (which the
code does). `read`/`write` snapshot the indices into locals so a mid-function ISR can't
shift the values they are comparing.

## How UART RX Uses It

`uart.c` now owns one ring buffer plus its backing array:

```c
static ring_buffer_t rb = {0U};
static uint8_t data_buffer[RING_BUFFER_SIZE] = {0U};   // 128 bytes
```

- `uart_setup()` calls `ring_buffer_setup(&rb, data_buffer, RING_BUFFER_SIZE)`.
- The ISR pushes each received byte: `ring_buffer_write(&rb, usart_recv(USART2))`.
- `uart_read()` drains up to `length` bytes and returns how many it got.
- `uart_data_available()` is now just `!ring_buffer_empty(&rb)`.

The old `data_available` flag is gone — it caused a read/write race between the ISR and
the main loop, and the ring buffer's index comparison replaces it.

## How To Test

Same as the 1B echo test, but now hold a key or paste a line of text: bytes typed
faster than the loop reads are queued and still echoed back in order, instead of being
overwritten.

```bash
screen /dev/ttyACM0 115200
```

Quit screen the right way so it releases the port: `Ctrl-A` then `k`, then `y`.

## Experiments

- Set `RING_BUFFER_SIZE` to a non-power-of-two (e.g. `100`) and watch the wrap break —
  proof of why the mask trick needs a power of two.
- Paste a chunk bigger than 127 bytes and confirm the tail is dropped (full buffer),
  while everything up to the limit still echoes.
- Add a counter in the `ring_buffer_write` failure path to actually count dropped bytes
  (the driver currently ignores the `false` return).
