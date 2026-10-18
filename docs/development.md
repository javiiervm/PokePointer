# Development and validation

PokePointer 1.0.0 is a **C++20 / Qt 6** console application for Windows 11 x64. The Windows backend reads the Poké Ball Plus over Bluetooth LE and generates native mouse events. This document is for contributors; most users should simply download the portable ZIP from [GitHub Releases](https://github.com/javiiervm/PokePointer/releases).

## Windows build prerequisites

- **Visual Studio 2022** or Build Tools 2022 with *Desktop development with C++*, MSVC v143 x64 tools, and the Windows SDK.
- **CMake 3.24+**.
- **Qt 6.5+**, installed with a compatible **MSVC x64** kit and the **Qt Connectivity / Bluetooth** module. CI uses Qt **6.12.0**, `msvc2022_64`. Do not use MinGW for the Windows BLE application.
- A Bluetooth Low Energy adapter for hardware tests (not needed by the unit tests).

Useful references: [Microsoft C++ tools](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation), [Qt installation](https://doc.qt.io/qt-6/get-and-install-qt.html), and [Qt on Windows](https://doc.qt.io/qt-6/windows.html). Confirm that Qt includes `lib/cmake/Qt6Bluetooth/Qt6BluetoothConfig.cmake`.

## Configure, compile, and test

Example for **Git Bash** with Qt at `C:\Qt\6.12.0\msvc2022_64`:

```bash
export PATH="/c/Qt/6.12.0/msvc2022_64/bin:$PATH"

cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 \
  -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
cmake --build build-msvc --config Release --parallel
ctest --test-dir build-msvc -C Release --output-on-failure

./build-msvc/Release/pokepointer.exe --version
```

A Visual Studio CMake generator finds the installed MSVC toolchain without requiring `cl.exe` on the ordinary Git Bash `PATH`. Use **a new build directory** if changing the generator or compiler. For local Debug builds, replace `Release` with `Debug` in the build, test, and executable paths.

The seven automated tests require no connected controller:

| Test | Main coverage |
| --- | --- |
| `core` | Input decoding, synthetic reports, connection-state policy, retry behavior, device selection |
| `pointer` | Calibration, deadzone, and relative cursor motion |
| `gestures` | Click, double-click, drag, scroll gesture, wheel mapping |
| `qt_offline` | Settings, identifier normalization, offline application behavior |
| `cli_help` | CLI help |
| `cli_invalid_combination` | Invalid scan/inspection combination |
| `cli_invalid_mouse_combination` | Invalid scan/mouse combination |

These tests do not validate a real BLE adapter, a physical controller, or all Windows shutdown/driver behaviors. Core-only builds (`-DPOKEPOINTER_BUILD_APP=OFF`) are possible without Qt, but **do not** validate Windows Bluetooth or mouse output.

## Run and diagnose

```bash
./build-msvc/Release/pokepointer.exe --mouse       # Native Windows mouse mode
./build-msvc/Release/pokepointer.exe               # Input diagnostics only
./build-msvc/Release/pokepointer.exe --scan        # List BLE advertisements, then exit
./build-msvc/Release/pokepointer.exe --inspect-gatt # Inspect services and characteristics
./build-msvc/Release/pokepointer.exe --debug-raw   # Rate-limited raw input previews
./build-msvc/Release/pokepointer.exe --help
```

Other useful options: `--device <address-or-UUID>` chooses a discovered device explicitly; `--forget-device` clears the app's remembered selection (not Windows pairing). `--mouse` cannot be combined with `--scan` or `--inspect-gatt`.

On first connection, keep the joystick centered and both buttons released for **12 consecutive stable input reports**. The log prints `Mouse calibrated` before pointer control starts. The selected device is saved in Qt's application settings, normally `%LOCALAPPDATA%\PokePointer\PokePointer\device.ini`.

A `streaming` state confirms notification subscription, not necessarily that valid input reports have arrived. If no notification arrives for 15 seconds, a warning is logged; this is **not** proof that Bluetooth disconnected. Mouse output is independently reset if valid reports stop for 500 ms. No vendor-specific keep-alive has been established.

When investigating connection problems, wake the controller, ensure Windows Bluetooth is on, and close other applications that may be connected to it. Use `--scan` and `--inspect-gatt` before `--debug-raw`. Qt Bluetooth debug logging can be enabled with `QT_LOGGING_RULES='qt.bluetooth*=true'` in the shell. Debug logs may reveal Bluetooth identifiers; do not post personal captures publicly without reviewing them.

## Stage a portable Windows package

After a successful **Release** build and test run:

```bash
cmake --install build-msvc --config Release \
  --prefix "$(cygpath -m "$PWD/dist/PokePointer-v1.0.0-windows-x64")"
```

The installed folder contains `Start-PokePointer.bat`, documentation and license notices, plus `bin/pokepointer.exe`, `Qt6Core.dll`, and `Qt6Bluetooth.dll`. CMake copies these two Qt DLLs directly: the current console application does not use Qt GUI/QML plugins. If its Qt dependencies change, revisit packaging and verify the runtime again.

To create a local ZIP:

```bash
cd dist
cmake -E tar cf PokePointer-v1.0.0-windows-x64.zip --format=zip PokePointer-v1.0.0-windows-x64
```

End users may need Microsoft's official **Visual C++ Redistributable x64**; the package deliberately does not bundle the compiler runtime. The workflow at [`.github/workflows/windows-release.yml`](../.github/workflows/windows-release.yml) configures and tests the Release build, stages the same files, smoke-tests `--version` without Qt's development folder on `PATH`, and uploads a ZIP. Pushing a matching `v*` tag causes it to publish a GitHub Release **after** the build job succeeds.

License material in `LICENSE` and `licenses/` is included in the ZIP, with optional Qt `qtbase` / `qtconnectivity` SBOM files when present in the Qt installation. See [Qt notices](../licenses/QT-NOTICE.md) and [third-party notices](../licenses/QT-THIRD-PARTY-NOTICES.md).

## Hardware release checklist

The v1.0.0 cursor, clicks, dragging, scroll gesture, console X/ Ctrl+C close, and a CI-produced ZIP were tested on Windows 11 with a Poké Ball Plus. Repeat the following checks when changing BLE or mouse code:

1. Discovery, GATT subscription, joystick directions, both button levels and center calibration.
2. Left/right click, double-click, dragging, scrolling in both directions, and no pointer drift at rest.
3. Shutdown with Ctrl+C and with the console X, **including during a drag**; verify no held mouse button or remaining `pokepointer.exe` process.
4. Unexpected BLE loss during a drag or scroll: verify output stops, held buttons release, and reconnection recalibrates successfully.
5. At least 15 minutes idle followed by input; document warnings or sleep behavior.
6. Try the downloaded ZIP on a separate Windows PC without Qt installed; test supported Bluetooth adapters and permissions.

**Not fully validated for v1.0.0:** prolonged idle, deliberate loss/reconnection, elevated applications and secure desktops, and portability across other machines. These are limitations to record, not reasons to claim the underlying protocol has been fully characterized.

## Project references

- [Mouse controls and timing](mouse.md)
- [Architecture](architecture.md)
- [Protocol evidence and limits](protocol.md)

The original Phase 0/1 setup notes described conditions before MSVC and Qt were installed in October 2026. Those historical limitations no longer describe the current build: Windows Release builds and the seven tests have since passed, and the mouse backend was tested with physical hardware.
