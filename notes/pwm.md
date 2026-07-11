# PWM Fade

PWM means pulse-width modulation. Instead of simply turning the LED on or off,
the timer turns it on and off very quickly. The human eye sees the average
brightness.

The current setup drives the onboard LED on `PA5` using `TIM2` channel 1.

## What We Did

The LED pin is no longer a normal GPIO output. It is switched to alternate
function mode:

```c
gpio_mode_setup(LED_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED_PIN);
gpio_set_af(LED_PORT, GPIO_AF1, LED_PIN);
```

For `PA5`, alternate function `AF1` connects the pin to `TIM2_CH1`.

The timer is configured for PWM mode:

```c
timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
timer_enable_oc_output(TIM2, TIM_OC1);
timer_enable_counter(TIM2);
```

Then the timer clock is divided and the period is chosen:

```c
#define PRESCALER 84
#define ARR_VALUE 1000
```

## Frequency

With an `84 MHz` timer clock:

```text
timer count frequency = 84,000,000 / 84 = 1,000,000 Hz
one count = 1 us
PWM period = 1000 counts = 1000 us = 1 ms
PWM frequency = 1000 Hz
```

General formula:

```text
PWM frequency = timer_clock / (prescaler * ARR)
```

## Duty Cycle

Duty cycle is the percentage of the period where the output is on:

```text
0%   = always off
25%  = on for 25% of each cycle
50%  = on half the time
100% = always on
```

The code converts percent into a compare value:

```c
ccr = ARR_VALUE * (duty_cycle / 100.0f);
```

Then:

```c
timer_set_oc_value(TIM2, TIM_OC1, ccr);
```

updates the PWM brightness.

## Current Fade Behavior

The main loop changes duty cycle every `10 ms`:

```c
duty_cycle += direction;
```

At `100%`, direction becomes negative. At `0%`, direction becomes positive.
That creates a breathing LED instead of a sudden jump from full brightness back
to off.

## Experiments

- Fixed brightness: call `timer_pwm_set_duty_cycle(5.0f)`, `25.0f`, `50.0f`,
  and `100.0f`.
- Fade speed: change the elapsed-time check from `10` to `1`, `20`, or `50`.
- Step size: change `direction` from `1.0f` to `0.25f` or `5.0f`.
- Flicker test: set `ARR_VALUE` to `50000`. The PWM becomes slow enough that
  flicker may become visible.
- Higher PWM frequency: set `ARR_VALUE` to `100`. The PWM becomes `10 kHz`, but
  you only get 100 brightness steps instead of 1000.

## Important Idea

`ARR` mostly controls frequency and resolution. `CCR` controls duty cycle.

```text
ARR = when the timer resets
CCR = when the output changes during that period
```
