<div align="center">
  <img width="650" alt="PokePointer Logo with Cursor" src="https://github.com/user-attachments/assets/74a23716-7ebf-4f50-838d-62b1030fb7f8" />
  <br />
  <p>
    <img src="https://img.shields.io/github/last-commit/javiiervm/PokePointer" alt="Last Commit" />
    <img src="https://img.shields.io/badge/platform-Windows%2011%20x64-lightgrey" alt="Windows 11 x64" />
    <img src="https://img.shields.io/badge/C%2B%2B-20-blue" alt="C++ Version" />
    <img src="https://img.shields.io/github/issues/javiiervm/PokePointer" alt="Issues" />
    <img src="https://img.shields.io/github/stars/javiiervm/PokePointer" alt="Stars" />
  </p>
</div>

**PokePointer** is a C++20 / Qt 6 application that **turns a Nintendo Poké Ball Plus into a wireless mouse for Windows 11.** Move the cursor, click, drag windows, and scroll using the controller's joystick and buttons over Bluetooth Low Energy (BLE).

> Linux and Android are not supported yet, but will be in future releases.

**Version 1.0.0** is the first Windows release. No installer or Qt development tools are needed to run the portable download.

## Download and use

1. Download **`PokePointer-v1.0.0-windows-x64.zip`** from [GitHub Releases](https://github.com/javiiervm/PokePointer/releases).
2. Extract the entire ZIP to a folder. Keep the `bin` directory and its DLLs together with the launcher.
3. Turn on Bluetooth, wake your Poké Ball Plus, and close other apps that may be connected to it.
4. Double-click **`Start-PokePointer.bat`** (not `pokepointer.exe`). A console window will open while PokePointer connects.
5. Keep the joystick centered and both buttons released until the console reports **`Mouse calibrated`**. You're ready to use it.

To stop, **close the console window with X** or press **Ctrl+C**. The close handler requests a clean shutdown, subject to Windows' shutdown time limits.

## Controls

| Action | Poké Ball Plus control |
| --- | --- |
| Move cursor | Tilt the joystick |
| Left click | Tap the top button |
| Double click | Tap the top button twice quickly |
| Drag | Hold the top button on the **first** press, then move the joystick |
| Right click | Press the joystick inward |
| Scroll up / down | Tap the top button, then quickly press and **hold it on the second press** while moving the joystick up / down |

For scrolling, begin the second press within **320 ms** of releasing the first, then hold it for approximately **300 ms**. Releasing the button returns to cursor movement. A single left click is delayed briefly so PokePointer can distinguish it from a double tap.

## Requirements and troubleshooting

- **Windows 11 x64**, a Bluetooth LE adapter, and a Nintendo Poké Ball Plus. Linux, macOS, and Android are not supported in this release.
- If the controller is not found, wake it, check that Bluetooth is enabled, and disconnect it from other apps. Discovery may take a few seconds.
- If Windows reports **`Qt6Core.dll`** or **`Qt6Bluetooth.dll`** as missing, extract the *whole* ZIP again; don't move the executable out of its `bin` folder.
- If a Microsoft Visual C++ runtime is missing, install the [official x64 redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe). Qt, Visual Studio, and CMake are **not** needed for normal use.
- Keep the joystick untouched during calibration. Windows may block mouse input injection into elevated applications or secure screens.

Mouse controls, scrolling, portable packaging, and console-window closing have been tested on Windows 11 with a physical controller. Extended idle behavior, unexpected disconnect/reconnect scenarios, and use on other Windows PCs have not been fully validated.

For gesture timing and validation details, see [Windows mouse controls](docs/mouse.md).

## Build on Windows

**Only developers building from source need** Visual Studio 2022 with the MSVC C++ workload, CMake 3.24+, and Qt 6.5+ with **Qt Core and Qt Bluetooth** from a compatible MSVC x64 kit. The project's release workflow uses Qt 6.12.0.

Example using **Git Bash** and Qt installed at `C:\Qt\6.12.0\msvc2022_64`:

```bash
export PATH="/c/Qt/6.12.0/msvc2022_64/bin:$PATH"

cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 \
  -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
cmake --build build-msvc --config Release --parallel
ctest --test-dir build-msvc -C Release --output-on-failure

./build-msvc/Release/pokepointer.exe --mouse
```

The default executable mode is **BLE diagnostics only**; `--mouse` enables mouse control. Run `--help` to see the diagnostic options. See [Development and validation](https://github.com/javiiervm/PokePointer/blob/main/docs/development.md) for building, packaging, testing, and troubleshooting.

## Documentation and license

- [Windows controls and gesture details](docs/mouse.md)
- [Development and validation](https://github.com/javiiervm/PokePointer/blob/main/docs/development.md)
- [Architecture](https://github.com/javiiervm/PokePointer/blob/main/docs/architecture.md) and [protocol evidence](https://github.com/javiiervm/PokePointer/blob/main/docs/protocol.md)

PokePointer is licensed under the [MIT License](LICENSE). The bundled Qt libraries have separate licenses; their terms, source information, and third-party notices are provided in [Qt notices](licenses/QT-NOTICE.md) and [third-party notices](licenses/QT-THIRD-PARTY-NOTICES.md). PokePointer is an independent project, not affiliated with Nintendo or The Qt Company.
