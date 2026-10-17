# Phase 2: Windows native mouse prototype

This opt-in prototype converts the existing verified Pokeball Plus input reports into native Windows relative mouse events with `SendInput`. It is not an HID driver and does not register an autostart process.

## Usage

1. Build with MSVC 2022 and Qt 6 Core/Bluetooth as described in README.md.
2. Wake the Poke Ball Plus. Close other applications connected to it.
3. From the project root, start `./build-msvc/Debug/pokepointer.exe --mouse` in Git Bash.
4. Keep the joystick centered and both buttons released until the `Mouse calibrated` message appears. Calibration requires twelve consecutive input samples within five units of the first sample.
5. Test movement over an empty desktop area; the top button maps to left mouse and the joystick press to right mouse. Holding a button permits dragging.
6. Stop with Ctrl+C. Diagnostics-only mode remains the default without `--mouse`.

For diagnostics, run with `--mouse --debug-raw`. `--mouse` cannot be used with `--scan` or `--inspect-gatt`.

## Design

- `core/PointerMotion.h`: Qt-free per-session center calibration, radial deadzone of 0.14 in normalized axis units, nonlinear speed scaling, subpixel carry and 50 ms maximum integration timestep. Approximate observed ranges on 2026-10-17 were X=42..191 and Y=41..180, with center near (118,114). Those observations inform initial axis scales; each new session still calibrates its center.
- `platform/WindowsMouseOutput.*`: uses Win32 `SendInput` relative mouse moves and button-level transitions. Repeated `held` reports do not generate repeated presses.
- `app/Application.*`: connects the packet decoder to the pointer mapper, drives a 16 ms timer only while moving, and releases injected button levels on disconnect, invalid report, Ctrl+C, shutdown or 500 ms without decoded reports. Initial calibration repeats after input invalidation.
- `--mouse` is opt-in; other modes retain their existing behavior.

Windows pointer speed/acceleration settings can affect physical pointer movement. Injected input may be blocked for elevated or secure desktops. This is not tested yet; do not treat software tests as hardware certification. There are no proprietary initialization or keep-alive writes.

## Manual test checklist (pending)

- Run `ctest --test-dir build-msvc -C Debug --output-on-failure`; ensure pointer and existing tests pass.
- Start without `--mouse`: confirm no cursor movement or mouse clicks.
- Start with `--mouse` and leave the joystick centered for calibration; confirm a stationary cursor for 30 seconds.
- Move slowly and fully left/right/up/down, then diagonally. Verify directions, speed scaling and smooth return to rest.
- Press and release the top button: exactly one left-button down and up.
- Press and release the joystick: exactly one right-button down and up.
- Hold the top button and move the joystick: drag an ordinary desktop window; release and ensure dragging stops.
- Hold both buttons and release one at a time; verify independent button states.
- Disconnect Bluetooth while holding a button: verify the injected button releases within 500 ms of the last valid report, then observe recovery.
- Re-enable Bluetooth and wake the controller. Verify a fresh connection, fresh center calibration and resumed controls.
- Press Ctrl+C while holding a mouse button. Verify the button releases and the process exits cleanly.
- Leave the controller untouched for at least 15 minutes, then verify controls respond and note any sleep/silence/reconnect behavior.
- Confirm lack of extra background processes or autostart entries.

If any Windows `SendInput` call is rejected, investigate privileges and pointer settings; do not work around secure-desktop restrictions.
