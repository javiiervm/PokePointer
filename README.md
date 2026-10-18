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

PokePointer is a C++20 / Qt 6 application that uses a Nintendo Poké Ball Plus as a wireless mouse. **Version 1.0.0 is the first stable Windows 11 x64 release**, providing cursor control, left/right clicks, double clicks, dragging, and vertical scrolling through `--mouse`. The default CLI mode remains BLE diagnostics. Linux and Android are not supported yet.

## Implemented

- BLE advertisement discovery and explicit or remembered device selection.
- Asynchronous connection, GATT discovery, and input notification subscription.
- GATT inspection with service, characteristic, and descriptor UUIDs and characteristic properties.
- Conservative decoding of coarse joystick values and both button levels from the known 17-byte input report.
- Optional timestamped raw hex previews, limited to ten lines per second with suppression counts.
- Operation deadlines, reconnect backoff (1, 2, 4, 8, 16, then 30 seconds), and fresh discovery on retry.
- Persistent selection in a local application settings file; `--forget-device` clears it.
- Ctrl+C / Ctrl+Break / console-window-close shutdown on Windows; SIGINT / SIGTERM handling on Unix.
- **Opt-in Windows mouse output:** joystick moves the cursor, a short top press performs a left click, holding top performs left-button dragging, and pressing the joystick performs a right click. Double-tap-and-hold top activates vertical scroll with the joystick.
- **Mouse safety:** initial center calibration, radial deadzone, speed curve, 500 ms input watchdog, button release on input invalidation and shutdown, and movement timer stopped at rest.
- Qt-free tests for protocol/lifecycle/pointer mapping plus Qt offline and CLI tests.

BLE discovery, GATT inspection, joystick decoding, mouse movement, left/right clicks, dragging, and the scrolling gesture were successfully tried with a physical Poké Ball Plus on Windows 11 on 2026-10-17. Seven tests passed in the MSVC Release build. **Recovery from unexpected Bluetooth loss, 15+ minute idle operation, elevated-app behavior, and the updated CI-built standalone distribution still require validation.** No proprietary keep-alive has been established. See [protocol evidence](docs/protocol.md) and [mouse validation](docs/mouse.md).

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

For a release build in the same configured build tree, use:

```bash
cmake --build build-msvc --config Release --parallel
ctest --test-dir build-msvc -C Release --output-on-failure
./build-msvc/Release/pokepointer.exe --version
```

The Visual Studio generator finds MSVC automatically; `cl.exe` does not need to be on Git Bash's PATH. Keep Qt's `bin` directory on PATH when running a build directly. See [development instructions](docs/development.md) for additional build details.

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
./build-msvc/Debug/pokepointer.exe --device "AA:BB:CC:DD:EE:FF" --mouse
```

`--mouse` cannot be combined with `--scan` or `--inspect-gatt`. It can be combined with `--debug-raw` for investigation. Press `Ctrl+C` to stop. Mouse mode is not installed as a driver, a background service, or a startup entry.

The first twelve stable button-free input reports calibrate the center. Keep the joystick untouched until `Mouse calibrated` appears. A radial deadzone suppresses small axis fluctuations. The joystick moves the cursor continuously with a nonlinear speed curve, up to approximately 900 pixels/second before Windows pointer processing. **Top button:** a short press is a left click (delayed by up to about 320 ms to recognize a second tap); holding the first press for 170 ms starts left-button dragging. Two short top presses generate a normal double click. To scroll, tap top once, then press and hold it again within 320 ms; after holding the second press for 300 ms, tilt the joystick vertically. Release top to return to normal cursor movement. This scroll gesture generates no clicks or dragging. **Joystick press = right click.** Mouse events are generated with the native Windows `SendInput` API.

If a decoded input report is missing for 500 ms, the mouse motion timer stops, any pending click is discarded, pressed buttons are released, and the pointer recalibrates on renewed input. BLE input invalidation and application shutdown also request button release. The normal Windows elevation/UIPI restrictions on injected input still apply. See [mouse validation](docs/mouse.md).

Selection is saved under Qt's `AppConfigLocation` (normally `%LOCALAPPDATA%\PokePointer\PokePointer\device.ini`). `--forget-device` clears the selection but does not remove Windows pairings. The app rescans and resubscribes after recoverable BLE failures. For details, see [development instructions](docs/development.md).

## Windows portable distribution

After building and testing **Release**, use CMake's install step to stage the Windows package. PokePointer currently uses **Qt Core and Qt Bluetooth only**, with no Qt GUI/QML modules or runtime Qt plugins. CMake therefore copies the release DLLs directly from the imported `Qt6::Core` and `Qt6::Bluetooth` targets, instead of invoking `windeployqt`. This avoids an observed `windeployqt` stall on GitHub Actions, while keeping the deployment small. If new Qt modules or plugins are introduced, update the deployment list and revalidate the package.

From Warp / Git Bash on the developer machine:

```bash
QT_ROOT="/c/Qt/6.12.0/msvc2022_64"
export PATH="$QT_ROOT/bin:$PATH"
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$(cygpath -m "$QT_ROOT")"
cmake --build build-msvc --config Release --parallel
ctest --test-dir build-msvc -C Release --output-on-failure
cmake --install build-msvc --config Release --prefix "$(cygpath -m "$PWD/dist/PokePointer-v1.0.0-windows-x64")"
```

The staged folder has this structure:

```text
PokePointer-v1.0.0-windows-x64/
  Start-PokePointer.bat
  README.md
  docs/mouse.md
  bin/
    pokepointer.exe
    Qt6Core.dll
    Qt6Bluetooth.dll
```

Double-click `Start-PokePointer.bat` to run `bin/pokepointer.exe --mouse`. The batch launcher works regardless of the installation directory and shows the exit code if the program fails. `Ctrl+C` in the console requests an orderly shutdown. **Closing the window with X requests orderly shutdown but remains subject to Windows console-close time limits**: the console handler waits up to four seconds for the Qt event loop to exit. Close-with-X behavior must be validated on real Windows; forced process termination cannot be handled.

The `.github/workflows/windows-release.yml` workflow builds Release with MSVC, runs seven tests, stages the same distribution, checks for both Qt DLLs, and runs `pokepointer.exe --version` without the Qt development `bin` folder on `PATH`. A successful workflow run uploads a downloadable ZIP artifact. Tagged builds can publish the ZIP as a GitHub Release; do not tag until the package is verified.

To create the archive locally, run from the repository root:

```bash
cd dist
cmake -E tar cf PokePointer-v1.0.0-windows-x64.zip --format=zip PokePointer-v1.0.0-windows-x64
```

Compiler runtime DLLs are deliberately not copied. End users might need the **official Microsoft Visual C++ Redistributable x64**. Test the package on Windows without Qt installed (and without its `bin` directory on `PATH`) and verify Bluetooth and all mouse gestures. Review Qt's licensing notices and redistribution obligations before publishing. The repository has not yet selected an application license.

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
| `docs/mouse.md` | Windows mouse behavior and manual validation |

## Next milestone

Validate the CI-built Windows ZIP without Qt installed, including all input gestures. Follow up with deliberate Bluetooth disconnection/reconnection, 15+ minute idle testing, and checks in elevated applications. Possible later improvements include configurable sensitivity and button mappings and Linux/Android backends. Gyroscope control is deferred.

No project license has been selected. Dependency licenses remain their owners' terms.
