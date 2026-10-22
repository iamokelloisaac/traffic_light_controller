# Traffic Light Controller

A dependency-free, POSIX-native traffic light state machine written in C23, with a
hand-rolled ANSI/UTF-8 terminal interface and mid-crossing pedestrian handling.

The project is intentionally small: it demonstrates how to structure a
non-trivial interactive C program with clean module boundaries, opaque types,
deterministic safety timing, and portable terminal I/O — while remaining
auditable by a single developer in one sitting.

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Safety Model](#safety-model)
4. [Architecture](#architecture)
5. [Directory Layout](#directory-layout)
6. [Building](#building)
7. [Running](#running)
8. [Interaction Model](#interaction-model)
9. [Module Reference](#module-reference)
10. [Design Decisions](#design-decisions)
11. [Portability](#portability)
12. [Testing](#testing)
13. [Troubleshooting](#troubleshooting)
14. [License](#license)

---

## Overview

The controller implements a pedestrian-crossing state machine:

```
   VEHICLE GREEN  +  PEDESTRIAN RED
            │
            │  (pedestrian button pressed)
            ▼
   VEHICLE YELLOW  +  PEDESTRIAN RED        (3 s)
            │
            ▼
   VEHICLE RED     +  PEDESTRIAN GREEN      (10 s)
            │
            ▼
   VEHICLE RED     +  PEDESTRIAN FLASHING   (5 s)
            │
            ▼
   VEHICLE GREEN   +  PEDESTRIAN RED
            │
            └──► loop until stopped manually
```

The state machine is fully event-driven. Every sleep is polled so that:

- A pedestrian button press is acted upon within one polling interval.
- A `q` keypress terminates the program cleanly from any state.
- A second pedestrian arrival during a crossing is either admitted
  (if the window is still open) or queued for the next cycle.

---

## Features

- **Deterministic safety timing** — Yellow = 3 s, walk = 10 s, flash = 5 s.
  These durations are constants in `timing.h` and are never shortened or
  extended in response to input.
- **Non-canonical input** — `termios` is used to deliver single keystrokes
  without requiring Enter. Restoration is automatic on `atexit` and `SIGINT`.
- **Mid-crossing handling** — A late pedestrian who presses the button while
  the pedestrian light is GREEN may join the current window; presses during
  yellow, flashing, or red are queued for the next cycle.
- **Opaque state type** — `TrafficLightState` is defined only inside
  `traffic_light.c`. Callers interact exclusively through getters and setters.
- **ANSI/UTF-8 UI** — Rounded box borders, coloured lamps, and bulleted logs.
  Windows 10+ is configured to interpret ANSI via `platform_init()`.
- **Zero runtime dependencies** — Standard C23 + POSIX 2008. No ncurses, no
  third-party library, no external configuration files.
- **Strict build** — `-Werror -Wpedantic -Wconversion` (and friends) are
  enabled by default. The code compiles warning-free.

---

## Safety Model

The state machine has three invariants that are enforced by construction:

1. **Vehicle and pedestrian never both show green.** The transition from
   vehicle GREEN → RED always passes through YELLOW (3 s) first.
2. **The pedestrian window is fixed.** Once the pedestrian light turns GREEN,
   the walk phase lasts exactly `PEDESTRIAN_CROSS_DURATION_SECONDS`. Input
   cannot shorten or extend it. A second pedestrian may *join* the current
   window but cannot change its remaining time.
3. **The flashing phase is a deadline, not an extension.** A pedestrian who
   presses the button during the flash phase is queued for the next cycle;
   the flashing phase always lasts exactly `PEDESTRIAN_FLASH_DURATION_SECONDS`.

All three invariants are visible in `run_crossing_sequence()` in
`src/state_machine.c` and are verified by reading the function top-to-bottom.

---

## Architecture

The program is split into nine modules. Each module owns a single concern and
communicates with others only through the public API in its header.

```
                       ┌──────────────┐
                       │    main.c    │
                       │   (entry)    │
                       └──────┬───────┘
                              │
        ┌──────────┬──────────┼──────────┬────────────┐
        │          │          │          │            │
        ▼          ▼          ▼          ▼            ▼
  ┌─────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌───────────┐
  │platform │ │terminal│ │  ui    │ │ timing │ │   input   │
  └─────────┘ └────────┘ └────────┘ └────────┘ └───────────┘
        │          │          │          │            │
        └──────────┴──────────┴────┬─────┴────────────┘
                                   │
                                   ▼
                     ┌───────────────────────────┐
                     │    state_machine.c        │
                     │ (flowchart orchestration) │
                     └────────────┬──────────────┘
                                  │
                                  ▼
                     ┌───────────────────────────┐
                     │     traffic_light.c       │
                     │ (opaque state lifecycle)  │
                     └───────────────────────────┘
```

**Dependency rules:**

- `traffic_light` depends on `ui` (for logging and rendering).
- `state_machine` depends on `traffic_light`, `input`, `timing`, and `ui`.
- `main` depends on `platform`, `terminal`, `state_machine`,
  `traffic_light`, and `ui`.
- `platform`, `terminal`, `timing`, and `input` are leaf modules with no
  project-internal dependencies.
- No module ever reaches into another module's `.c` file. Every cross-module
  call goes through a public header.

---

## Directory Layout

```
traffic_light_controller/
├── CMakeLists.txt
├── README.md
├── include/
│   └── traffic_light/
│       ├── input.h
│       ├── platform.h
│       ├── state_machine.h
│       ├── terminal.h
│       ├── timing.h
│       ├── traffic_light.h
│       └── ui.h
└── src/
    ├── input.c
    ├── main.c
    ├── platform.c
    ├── state_machine.c
    ├── terminal.c
    ├── timing.c
    ├── traffic_light.c
    └── ui.c
```

Headers live under `include/traffic_light/` so that call sites write
`#include "traffic_light/ui.h"` rather than a long relative path. This also
prevents header-file name collisions when the project is embedded into a
larger build.

---

## Building

### Prerequisites

| Requirement | Notes |
|-------------|-------|
| CMake ≥ 3.20 | Required for `target_compile_definitions`. |
| C23 compiler | GCC ≥ 13 or Clang ≥ 16. |
| POSIX 2008 | Linux, macOS, WSL. |
| Kali Linux | Default toolchain is sufficient. |

### Build steps

```bash
git clone <repo-url> traffic_light_controller
cd traffic_light_controller

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The binary is produced at:

```
build/traffic_light_controller
```

### Build types

| Type | Flags added by CMake |
|------|----------------------|
| `Debug` | `-g`, no optimization |
| `Release` | `-O3 -DNDEBUG` |
| `RelWithDebInfo` | `-O2 -g` |
| `MinSizeRel` | `-Os -DNDEBUG` |

The default build type, if none is specified, is `Debug`. The strict warning
set (`-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion
-Wsign-conversion`) is applied in every build type.

### Clean rebuild

```bash
cmake --build build --target clean
cmake --build build --parallel
```

---

## Running

```bash
./build/traffic_light_controller
```

On startup the terminal is cleared and a rounded box is drawn. The two
current lights are displayed on the line immediately beneath the box.

To trigger a pedestrian crossing, press **any key** (except `q`). To
terminate the program, press **`q`** or **`Ctrl+C`**. In both cases the
terminal is restored before exit.

### Example session

```
╭────────────────────────────────────────────────────────╮
│                 TRAFFIC LIGHT CONTROLLER               │
├────────────────────────────────────────────────────────┤
  │  VEHICLE  ● GREEN    │   WALK  ● RED
╰────────────────────────────────────────────────────────╯

------------------------- 
│ Waiting for pedestrian button...                       │
│ any key = press    q = stop                            │
-------------------------

  • Pedestrian button detected.

  • Starting crossing sequence
  • Vehicle light -> YELLOW
  │  VEHICLE  ● YELLOW   │   WALK  ● RED
  • Vehicle light -> RED
  • Pedestrian light -> GREEN
  │  VEHICLE  ● RED      │   WALK  ● GREEN
     • Pedestrians may cross (10s)
     • Additional pedestrian — join now (7 s left).
  • Pedestrian light -> FLASHING
  │  VEHICLE  ● RED      │   WALK  ● FLASHING
     • Pedestrian light flashing (5s)
  • Pedestrian light -> RED
  • Vehicle light -> GREEN
  │  VEHICLE  ● GREEN    │   WALK  ● RED
  • Crossing sequence complete
```

Colours in an ANSI-capable terminal: vehicle GREEN / pedestrian RED appear
in green and red respectively; FLASHING appears in cyan; log bullets match
the colour of the event they describe.

---

## Interaction Model

### Keys

| Key | Effect |
|-----|--------|
| Any key except `q`, `Q` | Simulates a pedestrian button press |
| `q` or `Q` | Requests a clean shutdown |
| `Ctrl+C` | Same as `q`; terminal is restored before exit |

### Behaviour when the button is pressed

The state machine polls for input every second. The effect of a press
depends on what the lights are currently showing:

| Vehicle light | Pedestrian light | Behaviour |
|---------------|------------------|-----------|
| GREEN | RED | Starts a new crossing sequence. |
| YELLOW | RED | Logs *"New pedestrian arrived early — please wait."*; queues them for the next cycle. |
| RED | GREEN | Logs *"Additional pedestrian — join now (N s left)."*; does **not** extend the window. |
| RED | FLASHING | Logs *"Too late to cross — queued for the next cycle."*; queues them. |
| RED | RED | Should not occur; if it does, treated as a button press and starts a new cycle. |

### Behaviour when the system is stopped

- If `q` is pressed while waiting for the pedestrian button, the current
  cycle is not started, and the program exits.
- If `q` is pressed mid-crossing, the current cycle's lights are left in
  the state they had reached, the terminal is restored, and the program
  exits. No cleanup delay is added.
- `Ctrl+C` takes the same path as `q`.

---

## Module Reference

### `traffic_light`

**Files:** `include/traffic_light/traffic_light.h`, `src/traffic_light.c`

Defines `TrafficLightState` as an **opaque type**: the struct is declared in
the header but defined only inside the implementation file. Callers can hold
a pointer to it, but cannot read or write its fields directly.

Public API:

| Function | Purpose |
|----------|---------|
| `traffic_light_create()` | Allocate a new state, both lights RED. |
| `traffic_light_destroy()` | Free the state. Safe to call with `NULL`. |
| `traffic_light_vehicle()` | Read current vehicle light. |
| `traffic_light_pedestrian()` | Read current pedestrian light. |
| `traffic_light_set_vehicle()` | Set vehicle light, log the change. |
| `traffic_light_set_pedestrian()` | Set pedestrian light, log the change. |
| `traffic_light_print_state()` | Render both lights on one line. |
| `light_state_to_string()` | Convert a `LightState` to a string. |

Ownership: whoever calls `traffic_light_create()` must also call
`traffic_light_destroy()`. In this project that is exclusively `main()`.

### `state_machine`

**Files:** `include/traffic_light/state_machine.h`, `src/state_machine.c`

Contains the flowchart logic. Consumes `TrafficLightState` only through
the opaque API.

Public API:

| Function | Purpose |
|----------|---------|
| `state_machine_run()` | Run the cycle until stopped; returns `true` on clean stop. |

Hidden internals (static functions in `state_machine.c`):

| Function | Purpose |
|----------|---------|
| `enter_initial_state()` | Set both lights to safe defaults. |
| `sleep_with_polling()` | Sleep, returning early on input. |
| `wait_for_pedestrian_or_stop()` | Wait indefinitely for button or `q`. |
| `run_crossing_sequence()` | Run one full crossing; may queue a follow-up. |

### `input`

**Files:** `include/traffic_light/input.h`, `include/traffic_light/input.c`

Non-blocking keystroke polling via `select()` with a zero timeout.

| Function | Purpose |
|----------|---------|
| `input_poll()` | Classify one keystroke as `INPUT_NONE`, `INPUT_BUTTON`, or `INPUT_STOP`. |
| `input_clear_buffer()` | Drain everything currently readable. |

The module never blocks and never consumes more than one keystroke per
call. Leftover bytes on the same line are drained so a burst of typing
cannot produce multiple events.

### `terminal`

**Files:** `include/traffic_light/terminal.h`, `include/traffic_light/terminal.c`

POSIX `termios` control.

| Function | Purpose |
|----------|---------|
| `terminal_enter_raw_mode()` | Disable `ICANON` and `ECHO`; register cleanup. |
| `terminal_restore()` | Restore the original settings; safe to call repeatedly. |

Cleanup is wired to `atexit` and `SIGINT`, so both normal exit and `Ctrl+C`
restore the terminal.

### `timing`

**Files:** `include/traffic_light/timing.h`, `include/traffic_light/timing.c`

| Function / Macro | Purpose |
|------------------|---------|
| `timing_sleep_seconds()` | EINTR-safe sleep for whole seconds. |
| `YELLOW_LIGHT_DURATION_SECONDS` | 3 |
| `PEDESTRIAN_CROSS_DURATION_SECONDS` | 10 |
| `PEDESTRIAN_FLASH_DURATION_SECONDS` | 5 |

`timing_sleep_seconds()` retries on `EINTR` so a signal cannot silently
shorten a phase.

### `ui`

**Files:** `include/traffic_light/ui.h`, `src/ui.c`

Formatting primitives. Knows nothing about traffic lights.

| Function | Purpose |
|----------|---------|
| `ui_clear_screen()` | Erase screen and home the cursor. |
| `ui_divider()` | Dim dashed rule. |
| `ui_box_border()` | One border row: `'T'`, `'M'`, or `'B'`. |
| `ui_box_line()` | Coloured, padded line inside a box. |
| `ui_box_title()` | Centred, bold, cyan title. |
| `ui_log()` | Bullet + coloured message. |
| `ui_both_lights()` | Two lamp indicators on one line. |

ANSI colour constants (`UI_RED`, `UI_GREEN`, `UI_YELLOW`, `UI_CYAN`,
`UI_RESET`, `UI_BOLD`, `UI_DIM`) are defined in the header so call sites
never write raw escape sequences.

### `platform`

**Files:** `include/traffic_light/platform.h`, `src/platform.c`

| Function | Purpose |
|----------|---------|
| `platform_init()` | Enable ANSI + UTF-8 on Windows 10+; no-op on POSIX. |

On Windows, this sets `ENABLE_VIRTUAL_TERMINAL_PROCESSING` on stdout and
`CP_UTF8` on the console code page. On Kali Linux, the function is empty.

### `main`

**File:** `src/main.c`

Owns the top-level lifecycle:

1. `platform_init()`
2. `terminal_enter_raw_mode()`
3. `traffic_light_create()`
4. `state_machine_run()`
5. `traffic_light_destroy()`
6. `terminal_restore()`
7. Print the farewell message and return.

Every resource allocated in `main()` is freed in `main()`. No hidden
singletons, no global state.

---

## Design Decisions

### Why an opaque type?

`TrafficLightState` is defined only in `traffic_light.c`. There are three
motivations:

1. **Invariant preservation.** Nothing outside the module can put the struct
   into an invalid state, because nothing outside can write to its fields.
   Setters are the only mutation path, and they can log every change.
2. **API stability.** Adding a field to the struct does not change any
   header or require recompiling any other file.
3. **Ownership clarity.** A pointer returned by `traffic_light_create()` is
   obviously meant to be passed to `traffic_light_destroy()`.

### Why hand-rolled UI instead of ncurses?

The UI needs are small: a box, a dashed rule, coloured text, and two
lamps. ncurses would add a build dependency and a heavier programming model
for very little gain. The hand-rolled implementation is 150 lines in
`ui.c`, understandable top-to-bottom, and portable to any ANSI/UTF-8
terminal.

The tradeoff is that older terminals (`TERM=dumb`, pre-Windows-10 consoles)
are not supported. That is acceptable for the target platforms (Kali and
Windows 10+).

### Why poll input during sleeps?

`sleep_with_polling()` and the loops in `run_crossing_sequence()` poll
every second. This is what makes it possible to:

- React to a second pedestrian within one second of their press.
- React to `q` immediately, rather than after the current phase ends.

A pure blocking sleep would freeze the UI for up to 10 seconds.

### Why not use signals for input?

Signals for keyboard input (like `SIGIO`) are fragile across terminal
emulators and do not integrate well with the `select()` model. Polling with
`select()` + a zero timeout is simple, portable, and predictable.

### Why `_POSIX_C_SOURCE 200809L`?

Strict C23 hides POSIX functions (`nanosleep`, `select`, `tcgetattr`,
`read`) behind feature-test macros. Defining `_POSIX_C_SOURCE 200809L` at
the top of every file that uses POSIX functions requests POSIX.1-2008,
which exposes exactly what we need without pulling in GNU-only extensions.

The define **must** precede every `#include` in that file. glibc evaluates
the macro when each header is first read.

### Why two entry files (`main.c` and `platform.c`) instead of one `main.c`?

`platform_init()` is called exactly once, at the start of the program. If
it lived in `main.c`, then `main.c` would need to `#include <windows.h>` on
Windows and nothing on POSIX, cluttering the entry point with `#ifdef`.
Keeping platform detection in its own module lets `main.c` stay free of
platform conditionals.

---

## Portability

The project targets:

| Platform | Support | Notes |
|----------|---------|-------|
| Kali Linux | Primary | Full support, ANSI + UTF-8 by default. |
| Other Linux | ✓ | Same as Kali, provided the terminal supports ANSI. |
| Windows 10+ | ✓ | `platform_init()` enables ANSI and UTF-8. `terminal.c` must be replaced with a Win32 implementation for raw mode. |
| WSL | ✓ | Behaves like Linux. |
| macOS | ✓ | Behaves like Linux, except `termios` flag ordering may vary slightly; tested on 12+. |
| Windows 7 / 8 | ✘ | No ANSI support; out of scope. |
| `TERM=dumb` | ✘ | Escape sequences will not be interpreted; out of scope. |

### Building on Windows

The current `terminal.c` uses POSIX `termios`, which is not available on
native Windows. To build natively:

1. Replace `include/traffic_light/terminal.c` with a Win32 version that calls
   `GetConsoleMode` / `SetConsoleMode` and stores the original console
   handle for restoration.
2. Keep `platform.c` as-is; it already enables ANSI and UTF-8.
3. The rest of the code compiles unchanged.

Alternatively, build inside WSL, where the POSIX implementation works as-is.

---

## Testing

The project does not currently ship automated tests, but the state machine
is structured so that it can be tested with a minimal fake for `input_poll`.
Suggested approach:

1. **Isolate the timing constants.** Replace the raw `sleep` calls in tests
   with a mock that records durations and returns immediately. The three
   durations (3, 10, 5) are asserted by inspecting the sequence of recorded
   sleeps.
2. **Stub `input_poll()`.** Feed a scripted sequence of `InputEvent` values
   (e.g. `{INPUT_BUTTON}`, `{INPUT_NONE} × 10`, `{INPUT_BUTTON}`) and assert
   that `run_crossing_sequence()` produces the expected sequence of light
   transitions.
3. **Assert mid-crossing behaviour.** Cover the three cases: press during
   yellow → queued; press during green → joined; press during flashing →
   queued.

The header boundary for `state_machine` is the natural seam: `run()` accepts
an opaque `TrafficLightState *`, so a test can substitute a fake with a
different `traffic_light_create()` if desired.

### Manual smoke test

```bash
./build/traffic_light_controller
```

1. Press **any key** — the yellow light should appear within one second.
2. Wait for the yellow phase to finish — the pedestrian light should turn
   green and stay green for exactly ten seconds.
3. During the green phase, press **any key** again — the program should
   print *"Additional pedestrian — join now (N s left)."* and the
   remaining time should still be ten seconds minus elapsed.
4. During the flash phase, press **any key** — the program should print
   *"Too late to cross — queued for the next cycle."* and the next cycle
   should start without requiring a fresh press.
5. Press **`q`** — the program should exit cleanly, and the shell prompt
   should appear on a new line.
6. Repeat step 5 but press **`Ctrl+C`** instead — same result.

---

## Troubleshooting

### "implicit declaration of function 'nanosleep'"

The compiler cannot see the POSIX prototype. Check that
`#define _POSIX_C_SOURCE 200809L` appears at the top of `include/traffic_light/timing.c`,
**before** any `#include`. The same applies to `include/traffic_light/terminal.c` and
`include/traffic_light/input.c` for `tcgetattr` / `select`.

### "Cannot resolve symbol 'state_machine_run'"

Either `#include "traffic_light/state_machine.h"` is missing from
`src/main.c`, or `state_machine.h` was accidentally emptied. Re-check the
include path in `include/traffic_light/CMakeLists.txt`:

```cmake
target_include_directories(traffic_light_controller
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

### Box borders look wrong / weird characters

The terminal is not interpreting UTF-8. Check the locale:

```bash
echo $LANG
```

It should contain `UTF-8` or `utf-8` (e.g. `en_US.UTF-8`). On Kali the
default is usually fine; if not, set:

```bash
export LANG=en_US.UTF-8
```

### Keys require Enter

The terminal is still in canonical mode. This happens if `terminal_enter_raw_mode()`
was not called, or if `termios` reported an error (e.g. stdin is not a
tty). Confirm you are running interactively, not via a pipe.

### Ctrl+C leaves the shell in a broken state

If `terminal_restore()` was bypassed (e.g. by `kill -9`), the terminal may
be left in raw mode. Restore it manually:

```bash
stty sane
```

### CMake says "C23 not supported"

Your compiler is too old. Upgrade GCC to 13+ or Clang to 16+, or lower the
standard in `include/traffic_light/CMakeLists.txt` (not recommended: the project uses `[[fallthrough]]`
and other C23 features).

---

## License

This project is released under the MIT License. See `LICENSE` for details.