# Development and validation

## Windows prerequisites and setup

The supported initial build is Windows 11 x64, MSVC 2022, CMake 3.24+, and Qt 6.5+ Core/Bluetooth from a compatible MSVC x64 kit. Consult the chosen Qt version's [Windows compiler compatibility](https://doc.qt.io/qt-6/windows.html). Do not use a MinGW kit for this application's Windows Bluetooth backend.

In Visual Studio Installer, modify the existing installation and select **Desktop development with C++**, the **MSVC v143 x64/x86 build tools**, and a **Windows SDK**. This is a manual system change requiring the owner's approval; no installer was run during project creation. The [Microsoft guide](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170) explains the workload.

Use the [Qt Online Installer](https://doc.qt.io/qt-6/get-and-install-qt.html) or Maintenance Tool to add a compatible MSVC x64 kit and Qt Bluetooth/Connectivity. Verify that the kit contains `lib/cmake/Qt6Bluetooth/Qt6BluetoothConfig.cmake`. Qt Creator is optional. No Qt Test dependency is required: offline tests use small executables and CTest.

The full PowerShell build/run commands are in [README.md](../README.md#build-on-windows). With the Visual Studio generator, CMake locates the compiler through the installed workload; `cl.exe` need not be on the ordinary shell's PATH. For a Ninja build, use an x64 Native Tools developer shell instead. Never change compilers in an existing CMake build directory.

For a Release build in the same configured Visual Studio build tree:

```powershell
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
.\build\Release\pokepointer.exe --scan
```

Keep the matching Qt `bin` directory on PATH in that shell. A missing Qt DLL at launch is a runtime search-path problem; an MSVC/Qt architecture mismatch requires reconfiguration with a matching kit. CMake's failure to find a Visual Studio instance can mean the C++ workload is absent even when the Visual Studio IDE is installed.

## Automated tests

| Test | Hardware access | Coverage |
| --- | --- | --- |
| `core` | None; Qt not linked | Published fixture, synthetic button states/holds, coarse axes, malformed reports, IMU isolation, bounded retry, lifecycle rules, device selection |
| `qt_offline` | None | Temporary-file settings persistence/forget, identifier normalization, idempotent stop-before-start |
| `cli_help` | None | CLI startup/help |
| `cli_invalid_combination` | None | Reject incompatible scan/inspection modes before radio access |

Tests use explicit checks that remain active in Release builds. They do not use assertion-only validation. No automated test proves Windows BLE behavior, callback ordering under real driver failures, physical packet correctness, or idle stability.

For core-only work when Qt is unavailable, configure a separate build with `-DPOKEPOINTER_BUILD_APP=OFF`. That target contains only standard C++, no Bluetooth or Qt. MSVC remains preferred on Windows; GCC/Clang can check this portable subset on other toolchains. Passing it does not validate the application.

The exact fallback commands used on this machine were:

```powershell
$env:PATH = 'C:\msys64\mingw64\bin;' + $env:PATH
cmake -S . -B build-core -G 'MinGW Makefiles' `
    -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe `
    -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe `
    -DPOKEPOINTER_BUILD_APP=OFF -DBUILD_TESTING=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

These commands build **only the portable core and its tests**. They must not be used to build the Windows Bluetooth application. Use a fresh PowerShell session for the MSVC/Qt commands to avoid mixing toolchain PATH entries.

## Initial validation record: 2026-10-16

- Repository initially contained only `.git`; no existing project source was overwritten.
- CMake 4.3.0 was available.
- Visual Studio Enterprise 2022 17.14 was installed, but `vswhere` found no installation with `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`; the expected MSVC tools directory was absent.
- No suitable Qt kit was found in PATH, Qt environment variables, or inspected standard installation locations. A custom installation elsewhere cannot be ruled out.
- Full configuration with `-G "Visual Studio 17 2022" -A x64` failed: CMake could not find a usable Visual Studio instance. The Qt-dependent application and tests were **not built or run**.
- An existing GCC 15.2.0 installation compiled the Qt-free core and tests successfully, with the configured warning flags and no reported warnings. The initial fallback configure needed its `bin` directory on PATH; after setting PATH it configured successfully.
- CTest core result: **1/1 passed**, covering decoder, retry policy, lifecycle transitions, and selection. These are software checks, not controller measurements.
- A separate configure check confirmed that enabling the Windows application with MinGW fails with the intended MSVC-only error before Qt discovery. No Bluetooth application was built with MinGW.
- No physical controller test, native mouse test, Linux run, or Android run was performed.

## Diagnostics and troubleshooting

1. Start with `--scan`. It lists all nearby BLE advertisements, not just Poke Balls. Wake the controller and close other applications that may hold the connection.
2. Use the printed address/UUID with `--device` if multiple candidates exist. Device names are not accepted as identifiers. Scan RSSI is an observation, not proof of identity.
3. Use `--inspect-gatt` before streaming. It lists UUIDs/properties without reading proprietary values or subscribing to unrelated characteristics. ATT handles are not exposed by this CLI; selection uses UUIDs.
4. Use `--debug-raw` to inspect the known input characteristic's bytes. Keep each joystick pose/button hold for several seconds because raw previews are sampled. Do not interpret suppression counts as Bluetooth packet loss.
5. `streaming` confirms a CCCD acknowledgment, not that valid reports have arrived. A silence warning is deliberately separate from a disconnect. There is no demonstrated keep-alive mechanism.
6. Recoverable errors in normal streaming mode rescan after backoff. Authorization/permission errors terminate so the OS condition can be corrected. A sleeping/nonadvertising peripheral may still require a physical wake.
7. The program does not automatically pair/unpair devices or clear the Windows GATT cache. Investigate OS settings only when observed errors justify it; record changes during testing.

For additional Qt transport logs in PowerShell:

```powershell
$env:QT_LOGGING_RULES = 'qt.bluetooth*=true'
.\build\Debug\pokepointer.exe --debug-raw
Remove-Item Env:\QT_LOGGING_RULES
```

Qt diagnostic output may include device identifiers. Keep local captures in the ignored `captures/` directory. No logs are uploaded by this application.

## Manual hardware validation

**All items below are pending.** Record date, Windows build, Qt version, adapter/driver, controller firmware if known, commands, observations, and elapsed time. Attach actual logs when marking an item passed.

| Check | Procedure | Expected evidence |
| --- | --- | --- |
| Discovery | Wake the Poke Ball; run `--scan` | Advertisement, name, identifier and RSSI; one-shot completion |
| Selection | Run with explicit identifier; then restart without it | Same identity selected; settings saved after subscription |
| Stale identity | Make saved identity unavailable; scan with a different eligible identity if available | Fresh scan; documented unique-candidate fallback, no guess among multiple devices |
| Explicit identity protection | Supply a valid but absent identifier | Repeated search for that identifier; no silent switch |
| GATT | Run `--device <id> --inspect-gatt` | Known service/characteristic and CCCD present; properties recorded; clean exit |
| Connection | Start default streaming | Ordered state changes through subscription acknowledgment; actual reports observed |
| Joystick | Rest, slowly move left/right/up/down and diagonals, return to rest | Raw coarse X/Y correlate with stick; record center, extrema and orientation |
| Motion isolation | Keep stick fixed and rotate/shake the controller | No interpretation of IMU bytes as joystick values; note real stick noise |
| Top button | Press, hold for 5 seconds, release | Only top level changes and remains held throughout |
| Stick press | Press, hold for 5 seconds, release | Only stick level changes and remains held throughout |
| Both buttons | Hold both, release one, release the other | Independent levels including both held; no invented click pulses |
| Idle stability | Leave connected untouched for at least 15 minutes, then exercise controls | Record silence warnings, link loss, report resumption and any wake requirement |
| Reconnection | Move out of range or disable/re-enable Bluetooth, then wake/return | State invalidation, bounded retries, fresh GATT/subscription, resumed input |
| Backoff | Keep the device absent through several attempts | Delays grow 1/2/4/8/16/30 seconds between attempt completion and the next scan |
| Shutdown stages | Ctrl+C during scan, connect, discovery, streaming and retry wait | One shutdown, no subsequent retries; exit within the disconnect deadline plus scheduling |
| Restart | Stop, then start again | No stale held-state output; no duplicate subscriptions from earlier runs |
| Forget | Run `--forget-device`, then restart | Saved selection absent; discovery runs again |

Dragging and native mouse movement/clicks are intentionally not tested in Phase 1 because no backend exists. Input holds can be validated now; OS dragging belongs to Phase 2.

## Next development steps

1. Install the missing MSVC workload and Qt kit, build the full application, and run all four CTest entries. Fix any compiler/backend-specific findings before hardware claims.
2. Complete discovery, GATT and button/joystick validation first. Preserve labeled evidence and add real fixtures without changing their provenance.
3. Resolve idle/reconnection behavior. Add an initialization or keep-alive only when its meaning and necessity are supported by evidence.
4. Add explicit calibration, configurable sensitivity/deadzone, and a Windows mouse backend. Define release-on-loss and hold/drag semantics before producing OS input.
5. Validate Linux/BlueZ separately, then add Android permission/package integration. Revisit gyroscope support only after the joystick path is reliable.
