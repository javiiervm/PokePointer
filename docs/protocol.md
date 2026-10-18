# Poké Ball Plus protocol: evidence and limits

PokePointer v1.0.0 reads joystick and button input from a Nintendo Poké Ball Plus over Bluetooth Low Energy. The input format is based on community reverse engineering and has been checked against the owner's physical controller on Windows 11. **This is not an official Nintendo protocol specification**, and it is not guaranteed to cover every controller firmware.

## Sources

- [rna0/pokeball-plus-4-windows](https://github.com/rna0/pokeball-plus-4-windows), especially its input-subscription and joystick-parsing routines.
- [Q-Bert-Reynolds' Poké Ball Plus gist](https://gist.github.com/Q-Bert-Reynolds/3ae3d151bd17cf5ac7bfc689159b716a), including the GATT tables and sample input reports.
- Qt's [Low Energy controller](https://doc.qt.io/qt-6/qlowenergycontroller.html) and [service](https://doc.qt.io/qt-6/qlowenergyservice.html) documentation for Bluetooth transport details (not for the proprietary packet layout).

## Known service and report fields

| Item | UUID or interpretation |
| --- | --- |
| Input service | `6675e16c-f36d-4567-bb55-6b51e27a23e5` |
| Input characteristic | `6675e16c-f36d-4567-bb55-6b51e27a23e6` (notify) |
| Client characteristic configuration descriptor (CCCD) | Standard `00002902-0000-1000-8000-00805f9b34fb` |
| Enable notifications | Write `01 00` to CCCD |
| Buttons, byte 1 | Bit 0 = top button, bit 1 = joystick press |
| Coarse joystick X | `(byte[3] & 0x0f) * 16 + (byte[2] >> 4)` |
| Coarse joystick Y | `byte[4]` |

Offsets are zero-based. The decoder accepts **17-byte reports** with no unsupported button-mask bits. Those conditions are a conservative implementation policy, not proof that no other report formats exist. Time and motion/IMU fields are ignored rather than misinterpreted as joystick data.

The original published sample used in unit tests is:

```text
0a 00 ec 86 7a d9 ff db 01 67 10 20 00 d0 ff e2 ff
```

It decodes to X=110, Y=122, with both buttons released. The additional test variants produced from it are **synthetic fixtures**, not physical captures. They test masks, holds, malformed reports, axis values, and isolation from ignored bytes.

## Windows hardware observations

On **2026-10-17**, BLE discovery, GATT access, joystick movement, both button levels, and native pointer control were observed with a physical controller. The coarse X values were approximately **42–191** and Y approximately **41–180**, with the neutral position near **X=118, Y=113–114**. Runtime calibration measures center from the current session instead of assuming those exact values for everyone.

The click, drag, and scroll gestures were subsequently tested in the Windows 1.0.0 implementation. This supports the chosen report fields and orientation on the tested device; it does not prove universal firmware compatibility or higher-resolution axes.

## Connection, idle, and unknown behavior

PokePointer scans for a fresh device advertisement, discovers the input service, and subscribes to notifications through the standard CCCD. It does **not** send undocumented vendor initialization packets, authentication sequences, rumble/LED commands, battery polling, or speculative keep-alive messages.

Recoverable connection failures trigger a fresh scan after bounded exponential backoff. No notifications for **15 seconds** produce a warning but do not necessarily indicate a disconnection. Separately, the mouse layer releases held buttons and resets calibration when valid input has been absent for **500 ms**.

The following remain uncertain: sleep and wake behavior after long idle periods, reconnection under deliberate radio loss, exact notification timing, fine-resolution joystick encoding, firmware differences, and OS-specific pairing/GATT-cache behavior. The application itself does not pair, unpair, or alter the Windows Bluetooth cache.

## Collecting further evidence

Use `--scan` for advertisements, `--inspect-gatt` for services/characteristics, and `--debug-raw` for rate-limited report previews. Hold each joystick pose long enough for sampled previews to appear. Duplicate suppression counts are **not** packet-loss measurements. Record date, firmware if known, Windows/Qt versions, input action, packet length, and raw bytes before making new protocol claims. Avoid sharing logs containing device identifiers unnecessarily.
