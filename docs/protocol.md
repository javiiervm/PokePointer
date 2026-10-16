# Protocol evidence and limits

Research date: 2026-10-16. Sources below are community reverse engineering, not a Nintendo specification. **Source-backed does not mean validated on the owner's physical controller.** Mutable upstream links may change.

## References

1. [rna0/pokeball-plus-4-windows](https://github.com/rna0/pokeball-plus-4-windows), particularly [PokeballController.cs](https://github.com/rna0/pokeball-plus-4-windows/blob/main/PokeballPlus4Windows/PokeballController.cs): `SubscribeToInputAsync`, `ParseAndForwardInput`, `GetAnalogX`, `GetAnalogY`, and initialization.
2. [Q-Bert-Reynolds' gist](https://gist.github.com/Q-Bert-Reynolds/3ae3d151bd17cf5ac7bfc689159b716a): `pbp_gatt_tables.txt`, `pokeball_plus.py` (`inputHandler` and captured reads), and `pbp_att_send_log.txt`.
3. Qt's [controller](https://doc.qt.io/qt-6/qlowenergycontroller.html), [service](https://doc.qt.io/qt-6/qlowenergyservice.html), and [Bluetooth overview](https://doc.qt.io/qt-6/qtbluetooth-index.html) describe the transport APIs, not the proprietary packet format.

## Source-backed fields

Both community references identify:

| Item | UUID / interpretation |
| --- | --- |
| Input service | `6675e16c-f36d-4567-bb55-6b51e27a23e5` |
| Input characteristic | `6675e16c-f36d-4567-bb55-6b51e27a23e6`, read + notify |
| CCCD | Standard `00002902-0000-1000-8000-00805f9b34fb`; notification enable value `01 00` |
| Buttons, byte 1 | Bit 0: top; bit 1: stick press; 0/1/2/3 represent released/top/stick/both |
| Coarse X | `(byte[3] & 0x0f) * 16 + (byte[2] >> 4)` |
| Coarse Y | `byte[4]`, unsigned |

Offsets are zero-based. The rna0 decoder uses those coarse values; the gist's unusual four-nibble X permutation has the same high eight bits. The gist uses bytes 4-5 for a finer Y value. Their calibration ranges differ, so PokePointer does not borrow either device's calibration or claim full-resolution axes.

The gist labels byte 0 as time and bytes from 6 onward as motion data. PokePointer ignores them. It never treats IMU samples as joystick movement.

## Validation policy and fixtures

The published read of input handle 65 is:

```text
0a 00 ec 86 7a d9 ff db 01 67 10 20 00 d0 ff e2 ff
```

This is the sole published 17-byte fixture used by the tests, not a local notification capture or a labeled center measurement. Our coarse interpretation is X=110, Y=122, both buttons released. Tests mutate it to exercise button masks, holds, releases, ignored bytes, all X byte combinations, all coarse Y values, and invalid lengths/flags. Mutations are synthetic tests, not experimental evidence.

The decoder accepts exactly 17 bytes and rejects button bits outside `0x03`. This is a conservative application policy based on the observed report, **not proof that every firmware emits exactly this length or that other bits can never occur**. Unknown reports remain visible through `--debug-raw`; they do not become fabricated controller state. The transport enforces characteristic provenance before decoding. Packet length and masks alone cannot authenticate arbitrary bytes, and no verified checksum is available.

## Initialization and idle behavior

The rna0 input path subscribes by writing the standard CCCD and shows no vendor initialization or periodic keep-alive. Its battery handling is not evidence of preventing sleep. The gist includes authentication-looking exchanges, vendor writes and light experiments, with notes reporting write failures. Their requirements and applicability to joystick streaming are unresolved. Replaying captured payloads would be speculation; PokePointer does not do it.

Initial behavior is therefore: connect, discover, enable input notifications, retain the connection, and reconnect on actual errors/disconnection. A silence warning reports the unresolved idle condition. No battery polling, authentication exchange, LED/sound command, rumble command, or proprietary keep-alive is sent.

## Hypotheses and unknowns to validate

- CCCD subscription alone may be sufficient for this controller/firmware; physical confirmation is still needed.
- Fine axis resolution and nibble packing need labeled joystick-only captures. Do not infer orientation from the controller's motion sensors.
- Axis center, range, direction, noise and deadzone must be measured on the owner's unit before Phase 2 normalization.
- Notification lengths, rates, unknown button flags and simultaneous button behavior may vary by firmware.
- Pairing/bonding requirements and Windows GATT cache behavior need testing. The application does not pair, unpair, or alter OS Bluetooth settings.
- Idle sleep timing, resumption after sleep, and whether any verified initialization/keep-alive is needed remain unknown. Reconnection cannot wake a sleeping peripheral that is not advertising.

Record firmware, OS/Qt versions, adapter, mode, input action, packet lengths and timestamps during validation. Raw previews are sampled: use slow, held poses initially. A future explicitly lossless capture facility may be necessary before reverse-engineering timing or fine axis data.
