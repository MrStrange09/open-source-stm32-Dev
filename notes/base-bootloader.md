# Base Bootloader

This checkpoint splits the firmware idea into two pieces:

```text
bootloader: small program that runs first
app: main firmware that the bootloader jumps into
```

For now, the bootloader does not update firmware yet. It only proves the basic
layout and jump flow.

## What We Did

A new `bootloader/` folder was added with its own Makefile, linker script, and
source file:

```text
bootloader/src/bootloader.c
bootloader/linkerscript.ld
bootloader/Makefile
```

The bootloader is limited to the first `32 KiB` of flash:

```ld
rom (rx) : ORIGIN = 0x08000000, LENGTH = 32K
```

The main app is expected to start immediately after that:

```c
#define BOOTLOADER_SIZE        0x8000U
#define MAIN_APP_START_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE)
```

So the app start address is:

```text
0x08000000 + 0x8000 = 0x08008000
```

## How The Jump Works

On Cortex-M, a firmware image begins with a vector table:

```text
offset +0: initial stack pointer
offset +4: reset handler address
```

The bootloader reads the app's reset handler from:

```c
MAIN_APP_START_ADDRESS + 4
```

Then it casts that address into a function pointer and calls it:

```c
typedef void (*void_fn)(void);
void_fn jump_fn = (void_fn)reset_vector;
jump_fn();
```

That means:

```text
bootloader starts
bootloader finds app reset handler
bootloader branches to app
app starts running
```

## Why Padding Exists

The bootloader must always occupy exactly the reserved region before the app.
If the raw bootloader binary is smaller than `32 KiB`, the app would otherwise
slide too early in flash.

`pad-bootloader.py` fills the rest of the bootloader binary with `0xff` until it
is exactly `0x8000` bytes.

That keeps this layout stable:

```text
0x08000000 - 0x08007FFF  bootloader
0x08008000 - ...         app
```

## How The App Includes The Bootloader

The app Makefile now builds this object:

```make
OBJS += $(SRC_DIR)/bootloader.o
```

That object comes from `app/src/bootloader.S`:

```asm
.section .bootloader_section
    .incbin "../bootloader/bootloader.bin"
```

The app linker script keeps that section at the beginning of flash:

```ld
KEEP (*(.bootloader_section))
```

So the final app image contains:

```text
padded bootloader binary
app vector table
app code
```

This lets one flashed image include both the bootloader and the current app.

## Build Order

Build the bootloader first:

```bash
cd bootloader
make
```

Then build the app:

```bash
cd ../app
make
```

The app build needs:

```text
../bootloader/bootloader.bin
```

because `bootloader.S` embeds that file.

## Important Gotchas

- The bootloader size constant and padding size must match.
- The app must really begin at the address the bootloader jumps to.
- If the bootloader grows beyond `32 KiB`, padding becomes negative and the
  layout is broken.
- A production jump usually also sets the vector-table offset and stack pointer.
  This checkpoint is just the first base jump experiment.

## Experiments

- Build bootloader, then app, and inspect that `bootloader.bin` is exactly
  `32768` bytes.
- Temporarily change `BOOTLOADER_SIZE` in the Python script and notice how the
  app layout assumption breaks.
- Use `arm-none-eabi-objdump -h firmware.elf` to inspect where sections land.
- Put a breakpoint in `bootloader.c` and another in the app `main()` to watch
  the jump happen in the debugger.
