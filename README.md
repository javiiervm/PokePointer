# PokePointer

PokePointer is a C++20 / Qt 6 CLI foundation for using a Nintendo Poke Ball Plus as a wireless mouse. **This version implements Phases 0 and 1 only: BLE communication and input diagnostics. It does not move or click the mouse.** Windows 11 x64 is the first target; Linux and Android remain future validation targets.

## Implemented

- BLE advertisement discovery and explicit or remembered device selection.
- Asynchronous connection, GATT discovery, and input notification subscription.
- GATT inspection with service, characteristic, and descriptor UUIDs and characteristic properties.
- Conservative, source-backed decoding of coarse joystick values and both button levels from the known 17-byte report. Values are uncalibrated; repeated input states are suppressed.
- Optional timestamped raw hex previews, limited to ten lines per second with suppression counts.
- Operation deadlines, reconnect backoff (1, 2, 4, 8, 16, then 30 seconds), and fresh discovery on retry.
- Persistent selection in a local application settings file; `--forget-device` clears it.
- Ctrl+C / Ctrl+Break shutdown on Windows; SIGINT / SIGTERM handling on Unix.
- Qt-free unit tests plus Qt offline and CLI tests for an installed Qt toolchain.

Implementation is not hardware certification. No physical-controller tests have been performed. No verified keep-alive has been established, so uninterrupted idle operation is an open validation item. See [protocol evidence](docs/protocol.md).

## Build on Windows

Install or modify these dependencies yourself; this repository does not install system software:

1. Visual Studio 2022 or Build Tools 2022 with **Desktop development with C++**, MSVC v143 x64/x86 tools, and a Windows SDK. An existing Visual Studio installation without that workload is insufficient. [Microsoft instructions](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170).
2. Qt 6.5 or newer, choosing a **compatible MSVC x64 kit**, with **Qt Core and Qt Bluetooth** (the Qt Connectivity module). Use a kit compatible with your installed MSVC version; for example, an `msvc2022_64` kit with VS 2022. Do not select MinGW. [Qt installation](https://doc.qt.io/qt-6/get-and-install-qt.html).
3. CMake 3.24 or newer. The Visual Studio generator uses MSBuild; Ninja is optional.
4. A working Windows BLE adapter, current adapter drivers, Bluetooth enabled, and a charged Poke Ball Plus.

Run in PowerShell. Enter the actual Qt kit directory when prompted; the example directory is not assumed to exist:

```powershell
Set-Location 'C:\Users\Javier\Documents\Personal\PokePointer'
$QtRoot = Read-Host 'Qt MSVC x64 kit directory (for example C:\Qt\6.8.3\msvc2022_64)'
if (!(Test-Path "$QtRoot\lib\cmake\Qt6Bluetooth\Qt6BluetoothConfig.cmake")) {
    throw 'This Qt kit does not contain Qt Bluetooth.'
}
$env:PATH = "$QtRoot\bin;" + $env:PATH
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$QtRoot"
cmake --build build --config Debug --parallel
ctest --test-dir build -C Debug --output-on-failure
```

Check each command succeeds before continuing. Do not mix Qt/compiler architectures or MSVC and MinGW libraries. CMake rejects non-MSVC Windows application builds. See [development instructions](docs/development.md) for troubleshooting and the validation record.

## Run

Use the same PowerShell session so Qt DLLs remain on `PATH`. Wake the controller with its top button and close other software using it.

```powershell
.\build\Debug\pokepointer.exe --help
.\build\Debug\pokepointer.exe --scan
.\build\Debug\pokepointer.exe --inspect-gatt
.\build\Debug\pokepointer.exe --debug-raw
.\build\Debug\pokepointer.exe
```

To choose a specific device, paste its identifier from `--scan`:

```powershell
$DeviceId = Read-Host 'Device identifier from --scan'
.\build\Debug\pokepointer.exe --device $DeviceId --inspect-gatt
.\build\Debug\pokepointer.exe --device $DeviceId --debug-raw
.\build\Debug\pokepointer.exe --forget-device
```

`--scan` lists nearby BLE devices for eight seconds and exits. `--inspect-gatt` connects once, inspects metadata, and exits without subscribing. Default mode and `--debug-raw` stream and retry until Ctrl+C. An explicit identifier is never replaced automatically. Without one, a missing cached identity can be replaced by the single advertised Poke Ball candidate; multiple candidates require explicit selection. Selection is saved only after notification subscription succeeds.

Settings are stored as `device.ini` under Qt's `AppConfigLocation` (normally beneath `%LOCALAPPDATA%\PokePointer\PokePointer` on Windows); the exact path is printed at startup. `--forget-device` alone clears the selection and exits. It can accompany another mode to clear the selection before running. It does not remove Windows pairings.

Decoded output such as `raw-x=110 raw-y=122 top=released stick=released` describes coarse device values, not screen coordinates. Both buttons can remain `held` simultaneously. Timestamp and IMU changes alone do not print new decoded states. Raw previews are intentionally sampled and are **not lossless captures**.

Exit codes: 0 for normal completion/shutdown, 1 for fatal BLE/inspection failure, 2 for validated CLI usage errors (Qt parser errors can return 1). Transient failures in streaming mode retry instead of exiting.

## Project map

| Location | Purpose |
| --- | --- |
| `src/main.cpp`, `src/app/` | CLI, coordination, output, device settings |
| `src/ble/` | Qt BLE transport and asynchronous lifecycle |
| `src/protocol/` | Stateless packet validation and decoding |
| `src/core/` | Input state, lifecycle rules, retry and device-selection policies |
| `src/platform/` | Console stop integration and future mouse-output contract |
| `tests/` | Hardware-independent tests; fixture provenance in protocol docs |
| `docs/architecture.md` | Ownership, data flow, lifecycle and platform boundaries |
| `docs/protocol.md` | Evidence, limitations, hypotheses and unknowns |
| `docs/development.md` | Toolchain, tests and manual hardware checklist |

## Next milestone

Complete the [manual hardware checklist](docs/development.md#manual-hardware-validation) before Phase 2. Then use measured center/range/orientation to add configurable sensitivity and deadzone, native movement, top-to-left and stick-to-right button mapping, held-button dragging, and guaranteed release on loss/shutdown. None of those mouse features or configuration controls is implemented yet. Gyroscope control is deferred.

No project license has been selected. Dependency licenses remain their owners' terms.
