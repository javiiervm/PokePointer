# PokePointer project rules

- Scope: C++20, Qt 6 Core and Bluetooth, CMake, and an asynchronous QCoreApplication CLI. Windows 11 x64 is the first target; keep Linux and Android possible. No GUI or native mouse output in Phases 0 and 1.
- Windows Bluetooth builds must use MSVC and a compatible Qt MSVC x64 kit. Never use MinGW for the Bluetooth application. A Qt-free core test build is separate from application validation.
- Separate BLE transport/lifecycle, protocol decoding, platform-independent state, future mouse backends, and application coordination. Keep protocol and core independent of Qt and OS APIs.
- Use explicit ownership, RAII, typed Qt signals/slots, and the event loop. Avoid blocking BLE calls, unnecessary threads, generic frameworks, and extra dependencies.
- Keep one active BLE attempt. Bound retry backoff and operation deadlines. Invalidate stale callbacks and input state after disconnect. Rediscover services and resubscribe on every connection. Shutdown must cancel retries.
- Persist device selection where possible. Rescan to obtain fresh device information; do not silently select among multiple candidates or replace an explicit identifier.
- Implement only protocol fields supported by evidence. Link sources and distinguish source-backed observations, hypotheses, and hardware verification. Never invent proprietary commands or treat battery reads as a proven keep-alive. Do not confuse IMU data with joystick input.
- Use English for code, comments, filenames, documentation, CLI output, and terminal messages.
- Prefer small C++20 value types, const correctness, scoped enums, std::span, and explicit validation. Preserve button levels for future holds/dragging rather than turning them into click events.
- Add meaningful hardware-independent tests for decoder validation, selection, retry policy, and lifecycle behavior. Label synthetic fixtures. Run available builds/tests and state precisely which checks could not run.
- Document actual implemented behavior and limitations. Never claim hardware checks passed without physical evidence. Keep the manual validation checklist current.
- Work only in this repository. No commits, branches, pushes, pull requests, repository creation, publishing, or other GitHub modifications without explicit user authorization. Do not modify other repositories or install system-wide dependencies without approval.
- Do not select a project license without asking the owner.
