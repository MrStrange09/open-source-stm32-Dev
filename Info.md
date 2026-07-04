# Learning Notes

Personal watch-along repo for the Low Byte Productions Bare Metal Series.

## Current Stage

- Board: Nucleo-F411RE
- MCU: STM32F411RE
- Debugger: onboard ST-Link
- Current working feature: SysTick-driven blinky on PA5

## Branch Policy

- `main`: latest current course progress.
- `blinky`: saved checkpoint where delay-loop blinky works.
- `systick-blinky`: saved checkpoint where SysTick blinky works.
- Later checkpoint branches: create only when a feature works, e.g. `systick`, `uart`.
- Keep `origin` as my GitHub repo.
- Keep `upstream` only as the original author's repo reference.

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

Run `make` from `app/`, not repo root.

```bash
cd app
make
```

Clean rebuild:

```bash
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

## SysTick Notes

- SysTick is configured for `1000 Hz`, so one tick is `1 ms`.
- `sys_tick_handler()` increments the global `ticks` counter.
- The main loop toggles the LED when `1000` ticks have passed.

Manual OpenOCD check:

```bash
openocd -s /usr/share/openocd/scripts -f interface/stlink.cfg -f target/stm32f4x.cfg
```

## Gotchas

- `arm-none-eabi-gcc` with no input prints `fatal error: no input files`; that is normal.
- `openocd` without `-f interface/... -f target/...` fails; that is normal.
- `stm32f4x.cfg` is correct for STM32F411RE.
- Use `gdb-multiarch -nx` for embedded debug so local GDB plugins like GEF do not interfere.
