# Blink

This was the first useful firmware behavior: turn the onboard LED on and off.
On the Nucleo-F411RE, the user LED is on `PA5`, so the code uses:

```c
#define LED_PORT GPIOA
#define LED_PIN  GPIO5
```

## What We Did

Before a GPIO pin can work, its peripheral clock must be enabled:

```c
rcc_periph_clock_enable(RCC_GPIOA);
```

Then the pin must be configured as an output:

```c
gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
```

After that, the LED can be changed with GPIO writes:

```c
gpio_toggle(LED_PORT, LED_PIN);
```

## Why It Works

GPIO registers are memory-mapped hardware registers. The code does not call an
operating system. It writes directly to STM32 peripheral registers.

`RCC_GPIOA` enables the clock gate for GPIOA. Without that clock, the GPIOA
hardware is not active. `GPIO_MODE_OUTPUT` changes the pin mode bits in the
GPIO mode register so `PA5` becomes an output pin.

## First Delay Method

The early blink used a busy-loop delay:

```c
for (uint32_t i = 0; i < cycles; i++) {
	__asm__("nop");
}
```

That works for learning, but it wastes CPU time. While the loop runs, the chip
does nothing useful. It also depends on compiler settings and clock speed, so it
is not a clean time source.

## Experiments

- Change the delay count and observe blink speed.
- Try `gpio_set()`, `gpio_clear()`, and `gpio_toggle()`.
- Remove the GPIO clock enable and see that the LED stops behaving correctly.
- Change `LED_PIN` to another pin and notice that the board LED no longer maps
  to your code unless hardware is actually connected there.
