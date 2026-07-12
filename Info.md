# Current Status

Personal watch-along repo for the Low Byte Productions Bare Metal Series.

## Current Stage

- Board: Nucleo-F411RE
- MCU: STM32F411RE
- Debugger: onboard ST-Link
- Current working feature: UART driver on USART2 (interrupt echo), RX now backed by a ring buffer (circular FIFO)
- Learning notes live in `notes/`.

## Branch Policy

- `main`: latest current course progress.
- `blinky`: saved checkpoint where delay-loop blinky works.
- `systick-blinky`: saved checkpoint where SysTick blinky works.
- `pwm`: saved checkpoint where timer PWM fade works.
- `baseBootloader`: saved checkpoint where the base bootloader builds and is embedded into the app image.
- `uart-driver-1b`: saved checkpoint where the USART2 UART driver echoes characters over the ST-Link virtual COM port.
- `ring-buffer`: saved checkpoint where UART RX is backed by a ring buffer so bursts of bytes are queued instead of dropped.
- Later checkpoint branches: create only when a feature works.
- Keep `origin` as my GitHub repo.
- Keep `upstream` only as the original author's repo reference.
- `libopencm3` is vendored into this repo so learning comments inside it are saved by normal Git commits.

Normal update:

```bash
git status
git add .
git commit -m "Short useful message"
git push
```

Create a working checkpoint:

```bash
git branch checkpoint-name
git push -u origin checkpoint-name
```

## Build

Build bootloader first, then app. The app embeds `bootloader/bootloader.bin`.

```bash
cd bootloader
make
cd ..
cd app
make
```

Clean rebuild:

```bash
cd bootloader
make clean
make
cd ..
cd app
make clean
make
```

## Debug

VS Code config:

```text
ST-Link: Debug Application
```

Flow: VS Code builds, OpenOCD talks to ST-Link, GDB connects, debugger stops at `main`; press `F5` to continue.

Manual OpenOCD check:

```bash
openocd -s /usr/share/openocd/scripts -f interface/stlink.cfg -f target/stm32f4x.cfg
```

## Gotchas

- `arm-none-eabi-gcc` with no input prints `fatal error: no input files`; that is normal.
- `openocd` without `-f interface/... -f target/...` fails; that is normal.
- `stm32f4x.cfg` is correct for STM32F411RE.
- Use `gdb-multiarch -nx` for embedded debug so local GDB plugins like GEF do not interfere.
- If app build cannot find `../bootloader/bootloader.bin`, build the bootloader first.
