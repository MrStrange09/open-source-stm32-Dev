# UART Driver (1B)

UART means universal asynchronous receiver/transmitter. It is a simple serial
link: one wire to send (`TX`), one wire to receive (`RX`), and no shared clock.
Both sides just agree on a speed (baud rate) ahead of time.

This checkpoint adds a first UART driver on `USART2` and uses it to echo
characters back to the PC terminal.

## Where The Driver Lives

The driver now lives in a shared area instead of inside the app only:

```text
shared/src/core/uart.c
shared/inc/core/uart.h
```

The idea is that both the app and (later) the bootloader can reuse the same
UART code.

## Pins And Peripheral

The onboard ST-Link exposes a virtual COM port over USB. On the Nucleo-F411RE
that virtual COM port is wired to `USART2`:

```c
#define UART_PORT (GPIOA)
#define TX_PIN    (GPIO2)   // PA2 -> USART2_TX
#define RX_PIN    (GPIO3)   // PA3 -> USART2_RX
```

The pins are switched to alternate function `AF7`, which connects them to
`USART2`:

```c
gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, TX_PIN | RX_PIN);
gpio_set_af(UART_PORT, GPIO_AF7, TX_PIN | RX_PIN);
```

## Configuration

`uart_setup()` configures the classic `115200 8N1` line and turns the peripheral
on last:

```c
usart_set_mode(USART2, USART_MODE_TX_RX);          // both directions
usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
usart_set_databits(USART2, 8);
usart_set_baudrate(USART2, 115200);
usart_set_parity(USART2, 0);                        // no parity
usart_set_stopbits(USART2, 1);

usart_enable_rx_interrupt(USART2);                  // fire IRQ on each byte
nvic_enable_irq(NVIC_USART2_IRQ);                   // let the NVIC deliver it

usart_enable(USART2);                               // enable AFTER config
```

`8N1` just means 8 data bits, No parity, 1 stop bit. Both ends must match, or
you get garbage characters.

## Transmit (Blocking)

Sending is simple and blocking. It waits for the transmit register to be free,
then writes one byte:

```c
void uart_write_byte(uint8_t data) {
    usart_send_blocking(USART2, (uint16_t)data);
}
```

`uart_write()` just loops over a buffer and calls `uart_write_byte()` for each
byte.

## Receive (Interrupt Driven)

Receiving does not poll. Instead the hardware raises an interrupt when a byte
arrives. libopencm3's vector table already expects a function named exactly
`usart2_isr`, so defining that function wires it to the `USART2` interrupt:

```c
void usart2_isr(void) {
    const bool overrun_occurred = usart_get_flag(USART2, USART_FLAG_ORE) == 1;
    const bool received_data    = usart_get_flag(USART2, USART_FLAG_RXNE) == 1;

    if (received_data || overrun_occurred) {
        data_buffer    = (uint8_t)usart_recv(USART2);  // reading clears the flag
        data_available = true;
    }
}
```

The received byte is parked in a tiny one-byte mailbox that the main loop can
check:

```c
static uint8_t data_buffer    = 0U;
static bool    data_available = false;
```

The read side is non-blocking:

```c
bool    uart_data_available(void);   // is there an unread byte?
uint8_t uart_read_byte(void);        // take the byte, clear the flag
```

## Echo Test In main()

The app proves both directions work by echoing every received byte back with
`+1`:

```c
if (uart_data_available()) {
    uint8_t data = uart_read_byte();
    uart_write_byte(data + 1);
}
```

So typing `a` in the terminal sends back `b`, `A` sends back `B`, and so on.
This exercises RX (interrupt path) and TX (blocking path) at the same time.

## How To Test

Flash the board, then open the virtual COM port:

```bash
screen /dev/ttyACM0 115200
```

Type a character and watch the next character come back. To quit screen the
right way (so it releases the port): `Ctrl-A` then `k`, then `y`.

## Important Limitations (Why This Is "1B")

> **Superseded:** these limitations are the reason for the next checkpoint. The
> single-byte mailbox has since been replaced by a ring buffer — see
> [ring-buffer.md](ring-buffer.md). The snippets below describe the original 1B
> code, kept here as history.

- The mailbox is only **one byte**. If two bytes arrive before the main loop
  reads, the first is silently overwritten. A ring buffer is the natural next
  step.
- `uart_read(data, length)` takes a `length` but copies at most **one** byte and
  returns `0` or `1`. It cannot honor `length` until there is a real buffer.
- `uart_read_byte()` does not check availability, so calling it when nothing
  arrived returns a stale byte. Gate it behind `uart_data_available()`.

## Serial Port Gotchas

- If `screen`/PuTTY says the port is **busy**, a previous detached `screen`
  session is probably still holding it. Fix:

  ```bash
  screen -ls                 # find the stale session
  screen -X -S <pid> quit    # or: pkill -f "SCREEN.*ttyACM0"
  screen -wipe
  ```

- `ModemManager` probes `/dev/ttyACM*` on plug-in and can grab the port or send
  stray bytes to the MCU. Stop it (`sudo systemctl stop ModemManager`) or add a
  udev rule that sets `ENV{ID_MM_DEVICE_IGNORE}="1"` for vendor `0483`,
  product `374b` (the ST-Link virtual COM port).
- A **charge-only** USB cable enumerates nothing. If `lsusb` shows no
  `STMicroelectronics ST-LINK`, swap the cable before debugging software.

## Experiments

- Echo without `+1` so the terminal shows exactly what you type.
- Send a fixed string on startup with `uart_write((uint8_t*)"hello\r\n", 7)`.
- Change the baud rate on one side only and watch it turn into garbage.
- Hold a key so bytes arrive faster than the loop reads, and observe the
  one-byte mailbox dropping characters (motivation for the ring buffer).
