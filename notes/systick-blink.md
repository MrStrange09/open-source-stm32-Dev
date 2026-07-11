# SysTick Blink

SysTick moved blinking away from a wasteful delay loop and toward time-based
logic. The CPU can keep running the main loop while time passes in the
background.

## What We Did

The system setup configures the main clock and starts SysTick:

```c
systick_set_frequency(SYSTICK_FREQ, CPU_FREQ);
systick_counter_enable();
systick_interrupt_enable();
```

With:

```c
#define CPU_FREQ     84000000
#define SYSTICK_FREQ 1000
```

SysTick interrupts `1000` times per second, so each tick is `1 ms`.

The interrupt handler increments a counter:

```c
void sys_tick_handler(void)
{
	ticks++;
}
```

The main loop checks elapsed time:

```c
if (system_get_ticks() - start_time >= 1000) {
	gpio_toggle(LED_PORT, LED_PIN);
	start_time = system_get_ticks();
}
```

## Why It Works

SysTick is a timer inside the Cortex-M core. It counts CPU clock cycles down to
zero. When it reaches zero, it can raise an interrupt and reload automatically.

The key idea is elapsed-time comparison:

```c
now - start_time >= delay
```

After toggling, `start_time` must be updated. If it is not updated, the
condition stays true forever after the first second, and the LED toggles too
fast to see clearly.

## Why `volatile`

The tick counter is changed inside an interrupt:

```c
static volatile uint64_t ticks;
```

`volatile` tells the compiler that `ticks` can change outside the normal flow of
the current function. Without it, the compiler might reuse an old value and not
read memory again.

## Experiments

- Change `1000` to `500` for a half-second blink.
- Change `1000` to `100` for a very fast blink.
- Comment out `start_time = system_get_ticks();` and observe the bug.
- Add another timed action with a different `start_time` variable.
