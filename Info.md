# Learning Notes

This repo is my hands-on workspace for following the Low Byte Productions Bare Metal Series.

## Current Stage: Blinky

Board: Nucleo-F411RE  
MCU: STM32F411RE  
Debugger: onboard ST-Link  
LED: PA5

## What We Changed

- Switched to the author repo's blinky-era branch: `my-blinky`.
- Updated VS Code debug config to use ST-Link + OpenOCD instead of J-Link.
- Set linker RAM to `128K` for STM32F411RE.
- Added `.vscode/gdb-multiarch-nx` so GDB ignores local GEF config during embedded debugging.

## Important Commands

Build firmware:

```bash
cd app
make
```

Clean and rebuild:

```bash
cd app
make clean
make
```

Check tools:

```bash
arm-none-eabi-gcc --version
gdb-multiarch --version
openocd --version
make --version
```

Test ST-Link/OpenOCD manually:

```bash
openocd -s /usr/share/openocd/scripts -f interface/stlink.cfg -f target/stm32f4x.cfg
```

Stop OpenOCD:

```bash
Ctrl+C
```

## Debugging In VS Code

Use this configuration:

```text
ST-Link: Debug Application
```

Expected flow:

1. VS Code runs `make bin` in `app`.
2. OpenOCD talks to the ST-Link.
3. GDB connects to OpenOCD.
4. The debugger stops at `main`.
5. Press `F5` / Continue to let the LED blink.

## Notes To Remember

- Run `make` from `app/`, not the repo root.
- `arm-none-eabi-gcc` with no input prints `fatal error: no input files`; that is normal.
- Running `openocd` without `-f interface/... -f target/...` fails; that is normal.
- `stm32f4x.cfg` is correct for STM32F411RE because F411 is part of the STM32F4 family.
- If GDB loads GEF, embedded debugging can fail with Python or remote I/O errors; use `gdb-multiarch -nx`.
