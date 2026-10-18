# Windows mouse controls (v1.0.0)

PokePointer uses a Nintendo Poké Ball Plus as a Bluetooth LE mouse on **Windows 11 x64**. Native mouse events are sent through the Windows `SendInput` API. No driver, GUI, background service, or startup entry is installed.

For a ready-to-run download and quick start, see the [README](../README.md).

## Start and stop

1. Enable Bluetooth, wake the Poké Ball Plus, and close other applications connected to it.
2. Extract the portable ZIP and double-click **`Start-PokePointer.bat`**. For a development build, run `pokepointer.exe --mouse` with Qt's runtime libraries on `PATH`.
3. Leave the joystick centered and both buttons **released** until `Mouse calibrated` appears in the console (12 stable input reports).
4. Use the controls below. Close the console with **X** or press **Ctrl+C** to stop.

Without `--mouse`, the program runs input diagnostics only. Normal shutdown releases held mouse buttons and stops BLE activity. The console X handler requests the same shutdown, but Windows imposes a close-event time limit; forced termination cannot be handled safely.

## Controls

| Action | Control |
| --- | --- |
| Move cursor | Tilt joystick |
| Left click | Briefly tap top button |
| Double click | Tap top button twice quickly |
| Drag | Hold top on the **first** press (at least 170 ms) and tilt joystick |
| Right click | Press joystick inward |
| Scroll vertically | Tap top once, quickly press **and hold the second press**, then tilt joystick up/down; release top to finish |

### How scroll differs from drag

- **Drag:** press the top button once and **keep that first press held**. Dragging starts after approximately **170 ms**.
- **Scroll:** tap and release top, then press it again **within 320 ms** and hold the second press for **300 ms**. While scrolling, the pointer stays still and joystick Y produces wheel events. Releasing top returns to cursor movement.
- **Double click:** release the second press *before* scroll activation to produce an ordinary double click.
- **Single click:** is deferred by about **320 ms** to rule out a second tap.

Scrolling uses standard 120-unit Windows wheel steps, up to approximately ten notches per second at maximum Y deflection. The radial deadzone and scroll deadzone suppress small fluctuations while the stick is centered.

## Calibration and safety

`PointerMotion` estimates center from twelve stable reports and uses a radial deadzone and nonlinear relative movement. A **500 ms watchdog** stops mouse output, releases held buttons, and resets calibration if valid reports stop. Invalid input or Bluetooth session invalidation also resets the mouse. This does not guarantee reconnection to a sleeping controller; there is no verified proprietary keep-alive.

Input injection may be restricted when interacting with elevated applications, UAC prompts, or secure desktops. PokePointer does not bypass Windows input security restrictions.

## Validation status

With a physical controller on Windows 11, movement, left/right clicks, double click, drag, vertical scrolling, Ctrl+C shutdown, and closing the console with X (including during dragging) were tested successfully. The portable ZIP compiled by GitHub Actions also worked on the same Windows PC. The seven automated tests pass in MSVC Release builds.

**Further tests recommended:** a continuous idle session of 15 minutes or longer; unexpected loss of Bluetooth during a drag and recovery after reconnection; launch on a different Windows PC without Qt installed; behavior with elevated applications and different Bluetooth adapters.

## Developer diagnostics

`--scan` lists nearby advertisements; `--inspect-gatt` shows services and characteristics; `--debug-raw` prints rate-limited input hex previews. `--mouse --debug-raw` is allowed, but `--mouse` cannot be used together with `--scan` or `--inspect-gatt`. The current controller selection is saved locally; `--forget-device` removes that app preference without unpairing Windows devices.

For detailed implementation and testing instructions, see [Development and validation](development.md), [Architecture](architecture.md), and [Protocol evidence](protocol.md).
