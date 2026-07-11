# Project Structure

The code is now split into smaller pieces instead of keeping everything in
`firmware.c`.

Current shape:

```text
app/src/firmware.c
app/src/core/system.c
app/src/core/timer.c

app/inc/core/system.h
app/inc/core/timer.h
```

## Why Split It

`firmware.c` should describe the application behavior:

```text
setup system
setup GPIO
setup timer
fade LED forever
```

`system.c` owns clock and SysTick setup.

`timer.c` owns TIM2/PWM setup.

This keeps each file focused. It also makes later features easier to add,
because UART, buttons, ADC, and other modules can each get their own source and
header files.

## What Headers Do

A header tells other `.c` files what functions exist:

```c
void system_setup(void);
uint64_t system_get_ticks(void);
```

Including a header does not compile the matching `.c` file. It only gives the
compiler declarations so function calls can be checked.

## What The Makefile Does

The include path lets the compiler find headers:

```make
DEFS += -I$(INC_DIR)
```

So this works:

```c
#include "core/system.h"
```

The object list tells Make which `.c` files become part of the firmware:

```make
OBJS += $(SRC_DIR)/$(BINARY).o
OBJS += $(SRC_DIR)/core/system.o
OBJS += $(SRC_DIR)/core/timer.o
```

That means:

```text
src/firmware.c      -> src/firmware.o
src/core/system.c   -> src/core/system.o
src/core/timer.c    -> src/core/timer.o
```

The linker then combines those object files into `firmware.elf`.

## Header Guard Gotcha

Each header needs a unique guard:

```c
#ifndef INC_TIMER_H
#define INC_TIMER_H
...
#endif
```

Copying `INC_SYSTEM_H` into `timer.h` made `timer.h` disappear after
`system.h` was included. That caused errors like:

```text
implicit declaration of function 'timer_setup'
```

The fix was to give `timer.h` its own guard name.
