<div align="center">
  <img width="650" alt="PokePointer Logo with Cursor" src="https://github.com/user-attachments/assets/74a23716-7ebf-4f50-838d-62b1030fb7f8" />
  <br />
  <p>
    <img src="https://img.shields.io/github/last-commit/javiiervm/PokePointer" alt="Last Commit" />
    <img src="https://img.shields.io/badge/platform-Windows-lightgrey" alt="Platform" />
    <img src="https://img.shields.io/badge/C%2B%2B-20-blue" alt="C++ Version" />
    <img src="https://img.shields.io/github/issues/javiiervm/PokePointer" alt="Issues" />
    <img src="https://img.shields.io/github/stars/javiiervm/PokePointer" alt="Stars" />
  </p>
</div>

PokePointer is a C++20 / Qt 6 CLI application for using a Nintendo Poke Ball Plus as a wireless mouse. **PokePointer 0.3 adds vertical wheel scrolling to the opt-in Windows mouse prototype via `--mouse`.** BLE diagnostics remain the default behavior. Windows 11 x64 is the first target; Linux and Android remain future validation targets.

## Implemented

- BLE advertisement discovery and explicit or remembered device selection.
- Asynchronous connection, GATT discovery, and input notification subscription.
- GATT inspection with service, characteristic, and descriptor UUIDs and characteristic properties.
- Conservative decoding of coarse joystick values and both button levels from the known 17-byte input report.
- Optional timestamped raw hex previews, limited to ten lines per second with suppression counts.
- Operation deadlines, reconnect backoff (1, 2, 4, 8, 16, then 30 seconds), and fresh discovery on retry.
- Persistent selection in a local application settings file; `--forget-device` clears it.
- Ctrl+C / Ctrl+Break shutdown on Windows; SIGINT / SIGTERM handling on Unix.
- **Opt-in Windows mouse output:** joystick moves the cursor, a short top press performs a left click, holding top performs left-button dragging, and pressing the joystick performs a right click. Double-tap-and-hold top activates vertical scroll with the joystick.
- **Mouse safety:** initial center calibration, radial deadzone, speed curve, 500 ms input watchdog, button release on input invalidation and shutdown, and movement timer stopped at rest.
- Qt-free tests for protocol/lifecycle/pointer mapping plus Qt offline and CLI tests.

Physical BLE discovery, GATT, joystick, and both button levels were observed on Windows 11 on 2026-10-17. **The PokePointer 0.2 Windows cursor and button output was physically tested successfully on 2026-10-17. The new PokePointer 0.3 scroll gesture has not yet been validated on Windows hardware.** Long idle stability, automatic reconnection, and behavior in elevated applications still require validation. No proprietary keep-alive has been established. See [protocol evidence](docs/protocol.md) and [mouse validation](docs/mouse.md).

## Build on Windows

Install or modify these dependencies yourself; this repository does not install system software:

1. Visual Studio 2022 or Build Tools 2022 with **Desktop development with C++**, MSVC v143 x64/x86 tools, and a Windows SDK. [Microsoft instructions](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170).
2. Qt 6.5 or newer, choosing a **compatible MSVC x64 kit**, with **Qt Core and Qt Bluetooth** (the Qt Connectivity module). Do not select MinGW. [Qt installation](https://doc.qt.io/qt-6/get-and-install-qt.html).
3. CMake 3.24 or newer and a working Windows BLE adapter.

Git Bash / Warp with Qt 6.12.0 and the existing `build-msvc` tree:

```bash
cd ~/Documents/Personal/PokePointer
QT_ROOT="/c/Qt/6.12.0/msvc2022_64"
export PATH="$QT_ROOT/bin:$PATH"
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$(cygpath -m "$QT_ROOT")"
cmake --build build-msvc --config Debug --parallel
ctest --test-dir build-msvc -C Debug --output-on-failure
```

The Visual Studio generator finds MSVC automatically; `cl.exe` does not need to be on Git Bash's PATH. Keep Qt's `bin` directory on PATH when launching the application. See [development instructions](docs/development.md) for additional build details.

## Run

The default CLI is still diagnostics-only:

```bash
./build-msvc/Debug/pokepointer.exe --help
./build-msvc/Debug/pokepointer.exe --scan
./build-msvc/Debug/pokepointer.exe --inspect-gatt
./build-msvc/Debug/pokepointer.exe --debug-raw
./build-msvc/Debug/pokepointer.exe
```

To enable native mouse output on Windows, **release the joystick while connecting and calibrating**, and run:

```bash
./build-msvc/Debug/pokepointer.exe --mouse
```

To explicitly select the controller:

```bash
./build-msvc/Debug/pokepointer.exe --device b8:8a:ec:e6:a6:4a --mouse
```

`--mouse` cannot be combined with `--scan` or `--inspect-gatt`. It can be combined with `--debug-raw` for investigation. Press `Ctrl+C` to stop. Mouse mode is not installed as a driver, a background service, or a startup entry.

The first twelve stable button-free input reports calibrate the center. Keep the joystick untouched until `Mouse calibrated` appears. A radial deadzone suppresses small axis fluctuations. The joystick moves the cursor continuously with a nonlinear speed curve, up to approximately 900 pixels/second before Windows pointer processing. **Top button:** a short press is a left click (delayed by up to about 320 ms to recognize a second tap); holding the first press for 170 ms starts left-button dragging. Two short top presses generate a normal double click. To scroll, tap top once, then press and hold it again within 320 ms; after holding the second press for 300 ms, tilt the joystick vertically. Release top to return to normal cursor movement. This scroll gesture generates no clicks or dragging. **Joystick press = right click.** Mouse events are generated with the native Windows `SendInput` API.

If a decoded input report is missing for 500 ms, the mouse motion timer stops, any pending click is discarded, pressed buttons are released, and the pointer recalibrates on renewed input. BLE input invalidation and application shutdown also request button release. The normal Windows elevation/UIPI restrictions on injected input still apply. See [mouse validation](docs/mouse.md).

Selection is saved under Qt's `AppConfigLocation` (normally `%LOCALAPPDATA%\PokePointer\PokePointer\device.ini`). `--forget-device` clears the selection but does not remove Windows pairings. The app rescans and resubscribes after recoverable BLE failures. For details, see [development instructions](docs/development.md).

## Project map

| Location | Purpose |
| --- | --- |
| `src/main.cpp`, `src/app/` | CLI, coordination, output, device settings |
| `src/ble/` | Qt BLE transport and asynchronous lifecycle |
| `src/protocol/` | Stateless packet validation and decoding |
| `src/core/` | Input state, pointer mapping, lifecycle, retry and selection policies |
| `src/platform/` | Console stop, mouse-output contract, Windows `SendInput` backend |
| `tests/` | Hardware-independent tests; fixture provenance in protocol docs |
| `docs/architecture.md` | Ownership, data flow, lifecycle and platform boundaries |
| `docs/protocol.md` | Evidence, limitations, hypotheses and unknowns |
| `docs/development.md` | Toolchain, tests and manual hardware checklist |
| `docs/mouse.md` | Phase 2 behavior and manual Windows mouse validation |

## Next milestone

Compile and validate the new scroll gesture on Windows hardware, particularly short clicks, double clicks, drag, scroll direction/speed, and release without stray clicks. Then verify reconnection and long-term stability, consider configurable sensitivity and button mappings, and eventually Linux/Android backends. Gyroscope control is deferred.

No project license has been selected. Dependency licenses remain their owners' terms.
