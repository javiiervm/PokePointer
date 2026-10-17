# Phase 2: Windows mouse and wheel control

PokePointer 0.3 adds wheel events to the opt-in native Windows mouse prototype. It uses Win32 `SendInput`, not an HID driver, background service, or startup entry. PokePointer 0.2 cursor movement, both buttons, and a clean shutdown were physically demonstrated on 2026-10-17. The 0.3 scrolling gesture needs on-device validation.

## Usage

1. Build with MSVC 2022 and Qt 6 Core/Bluetooth as described in README.md.
2. Wake the Poke Ball Plus and close other software using its Bluetooth connection.
3. In Warp/Git Bash run `./build-msvc/Debug/pokepointer.exe --mouse`.
4. Leave the joystick centered and both buttons released until `Mouse calibrated` appears; calibration uses twelve consecutive stable input reports.
5. Test movement, short left click, short double click, long-press dragging, right click, and finally scrolling on a page.
6. Press `Ctrl+C` in the terminal to stop; normal diagnostics remain the default without `--mouse`.

## Mapping and gesture timing

| Action | Poké Ball Plus control |
| --- | --- |
| Move cursor | Tilt joystick |
| Left click | Quickly press and release top once (tap) |
| Double click | Tap top twice quickly |
| Drag | Hold top from the **first** press, then tilt joystick |
| Right click | Press joystick inward |
| Scroll up/down | Tap top once, tap it again within 320 ms, **hold the second press for 300 ms**, tilt joystick up/down; release top to exit |

- A first short top tap is **delayed by at most approximately 320 ms** while the app waits for a possible second tap. This is a trade-off for avoiding unintended clicks during the scroll gesture.
- On an ordinary first press, the left mouse button is held after 170 ms; moving the joystick then drags windows. The stick button remains an independent right-click button.
- A second press suppresses cursor movement while it is being interpreted. A short second press results in two clicks (standard double click). A held second press activates scrolling and **generates neither click nor drag**. During scroll, the X axis is ignored and the Y axis emits vertical wheel notches.
- Scroll is generated via native `MOUSEEVENTF_WHEEL` events in standard 120-unit steps, up to approximately ten steps per second at maximum deflection. A deadzone avoids accidental scrolling at rest.
- The app uses one active 16 ms timer for cursor motion, gesture deadlines, or scrolling; it stops at rest. No extra worker threads, services or dependencies were added.
- `--mouse --debug-raw` remains available. `--mouse` cannot be combined with `--scan` or `--inspect-gatt`.

## Safety and implementation

- `core/PointerMotion.h` remains unchanged from PokePointer 0.2: session center calibration, radial deadzone, nonlinear pointer speed and capped integration step.
- `core/TopButtonGesture.h` is a Qt-free state machine for deferred single clicks, ordinary held dragging, double clicks, and double-tap-hold scrolling. A disconnect or invalid input resets it, discarding pending taps rather than injecting them later.
- `core/ScrollMotion.h` maps the calibrated Y displacement into wheel notches, independently of cursor motion.
- `platform/MouseOutput.h` adds a `scrollVertical` interface, implemented by `platform/WindowsMouseOutput.cpp` through native Windows `SendInput`.
- `app/Application.cpp` stops cursor movement immediately when the second tap begins, switches to wheel events after 300 ms, and releases mouse buttons on shutdown, BLE invalidation, unknown report, or 500 ms without valid input. A watchdog-triggered reset also forces fresh calibration.

As in PokePointer 0.2, input injection can be blocked on elevated or secure desktops by Windows protections. There are no proprietary controller writes or keep-alive commands.

## Manual validation on Windows (pending for v0.3)

1. Build with MSVC; run `ctest --test-dir build-msvc -C Debug --output-on-failure` and confirm all seven tests pass.
2. Start without `--mouse`: ensure diagnostics work and the cursor is unchanged.
3. Start with `--mouse`; calibrate with the joystick untouched, then test all four movement directions and no drift at rest.
4. Tap top once over a safe target. Confirm exactly one click, despite the brief recognition delay.
5. Tap top twice quickly. Confirm a double click (such as selecting a word) and no scroll.
6. Hold top on its first press for at least 170 ms and drag a window. Verify releasing it ends the drag.
7. Hover over a scrollable web page. Tap and release top briefly, press it again within 320 ms and **keep the second press held for at least 300 ms**. Move joystick down, then up. Verify the page scrolls both ways, the pointer stays still and there is no text selection, drag, or unintended click. Release top and verify pointer movement resumes.
8. Test releasing the second press before 300 ms; it must act as a double click rather than entering scroll mode.
9. Press the joystick for a right click and verify it still works independently.
10. Disconnect the controller during a drag or scroll: events must stop within 500 ms, held buttons must release, and pending clicks must be discarded. Reconnect and verify recalibration.
11. Stop with `Ctrl+C` while holding the button, then check that no click remains held.
12. Later, check 15-minute idle stability and auto-reconnection.

Check the actual scroll rate in browser and Windows Explorer before calling this mapping final; adjustment may be needed for different apps.
