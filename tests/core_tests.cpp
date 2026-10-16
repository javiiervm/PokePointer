#include "core/Backoff.h"
#include "core/ConnectionState.h"
#include "core/DeviceSelection.h"
#include "protocol/InputDecoder.h"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void decoderTests()
{
    using namespace pokepointer;
    // Published read of handle 65 in Q-Bert-Reynolds' gist, NOT a local hardware capture.
    // Source and fixture provenance are documented in docs/protocol.md.
    std::array<std::uint8_t, 17> sample{
        0x0a, 0x00, 0xec, 0x86, 0x7a, 0xd9, 0xff, 0xdb, 0x01,
        0x67, 0x10, 0x20, 0x00, 0xd0, 0xff, 0xe2, 0xff};
    auto result = decodeInput(sample);
    check(result.state.has_value(), "Published 17-byte sample must decode");
    check(*result.state == InputState{110, 122, false, false}, "Published sample coarse axes/buttons");

    // Synthetic mutations exercise the source-backed button bit meanings and held levels.
    for (std::uint8_t buttons = 0; buttons < 4; ++buttons) {
        sample[1] = buttons;
        result = decodeInput(sample);
        check(result.state->topPressed == ((buttons & 1) != 0), "Top button mask");
        check(result.state->stickPressed == ((buttons & 2) != 0), "Stick button mask");
        check(result.state == decodeInput(sample).state, "Repeated reports preserve held levels");
    }
    sample[1] = 0;
    const auto stable = decodeInput(sample).state;
    sample[0] ^= 0xff;
    for (std::size_t i = 6; i < sample.size(); ++i) sample[i] ^= 0xff;
    check(decodeInput(sample).state == stable, "Timestamp and IMU must not change joystick/button state");
    sample[2] ^= 0x0f;
    sample[3] ^= 0xf0;
    sample[5] ^= 0xff;
    check(decodeInput(sample).state == stable, "Unverified fine axis bits are not decoded");

    // Exhaust all byte-pair combinations, independently checking the two X nibbles.
    for (unsigned int pair = 0; pair < 65536; ++pair) {
        sample[2] = static_cast<std::uint8_t>(pair >> 8);
        sample[3] = static_cast<std::uint8_t>(pair);
        const auto expected = static_cast<std::uint8_t>((pair % 16) * 16 + pair / 4096);
        check(decodeInput(sample).state->joystickX == expected, "X nibble ordering");
    }
    for (unsigned int value = 0; value < 256; ++value) {
        sample[4] = static_cast<std::uint8_t>(value);
        check(decodeInput(sample).state->joystickY == value, "Y is an unsigned coarse value");
        sample[1] = static_cast<std::uint8_t>(value);
        const auto decoded = decodeInput(sample);
        check(decoded.state.has_value() == (value < 4), "Reserved button bits rejected");
        if (value >= 4) check(decoded.error == DecodeError::UnsupportedButtonBits, "Button validation error");
        sample[1] = 0;
    }
    for (std::size_t length = 0; length < 80; ++length) {
        std::vector<std::uint8_t> packet(length);
        const auto decoded = decodeInput(packet);
        check(decoded.state.has_value() == (length == 17), "Unknown lengths must not decode");
        if (length != 17) check(decoded.error == DecodeError::UnsupportedLength, "Length validation error");
    }
}

void backoffTests()
{
    pokepointer::Backoff backoff;
    for (const auto milliseconds : {1000, 2000, 4000, 8000, 16000, 30000, 30000})
        check(backoff.next().count() == milliseconds, "Bounded exponential retry sequence");
    for (int i = 0; i < 10000; ++i)
        check(backoff.next().count() == 30000, "Backoff must not overflow after long failures");
    backoff.reset();
    check(backoff.next().count() == 1000, "Stable-session reset");
}

void lifecycleTests()
{
    using namespace pokepointer;
    using enum ConnectionState;
    Lifecycle lifecycle;
    check(!lifecycle.transition(Streaming), "Cannot stream before discovery/subscription");
    for (auto state : {Scanning, Connecting, Discovering, Subscribing, Streaming}) {
        check(lifecycle.transition(state), "Normal lifecycle path");
        check(!lifecycle.transition(state), "Overlapping/repeated transition rejected");
    }
    check(!lifecycle.transition(Connecting), "Cannot start another connection while streaming");
    check(lifecycle.transition(Reconnecting), "Disconnect schedules retry");
    check(!lifecycle.transition(Connecting), "Retry requires fresh scan");
    check(lifecycle.transition(Scanning), "Retry starts scan");
    check(lifecycle.transition(ShuttingDown), "Shutdown during scan");
    check(!lifecycle.transition(Reconnecting), "Shutdown cancels reconnect");
    check(lifecycle.transition(Stopped), "Shutdown completes");
    check(!lifecycle.transition(Scanning), "Stopped session never restarts");

    const std::array active{Scanning, Connecting, Discovering, Subscribing, Streaming};
    for (std::size_t failure = 0; failure < active.size(); ++failure) {
        Lifecycle session;
        for (std::size_t i = 0; i <= failure; ++i)
            check(session.transition(active[i]), "Prepare failure stage");
        check(session.transition(Reconnecting), "Every asynchronous stage can fail");
        check(!session.transition(Reconnecting), "Duplicate failure cannot schedule another retry");
        check(session.transition(ShuttingDown), "Shutdown during backoff");
        check(session.transition(Stopped), "Complete shutdown during backoff");
    }
    Lifecycle idle;
    check(idle.transition(ShuttingDown), "Shutdown before start");
    check(idle.transition(Stopped), "Idle shutdown completion");
}

void selectionTests()
{
    using namespace pokepointer;
    std::vector<DeviceCandidate> devices{{"a", true}, {"b", false}, {"c", true}};
    check(selectDevice(devices, "", "").status == SelectionStatus::Ambiguous, "Never guess among devices");
    check(selectDevice(devices, "", "c").index == 2, "Prefer cached identity among candidates");
    check(selectDevice(devices, "a", "c").index == 0, "Explicit identity overrides saved identity");
    check(selectDevice(devices, "b", "").index == 1, "Explicit inspection permits unfamiliar device names");
    check(selectDevice(devices, "missing", "a").status == SelectionStatus::NotFound,
          "Never silently replace an explicit device");
    devices.pop_back();
    const auto replacement = selectDevice(devices, "", "old");
    check(replacement.index == 0 && replacement.replacedCachedIdentity, "Rescan can replace stale cached identity");
    devices.erase(devices.begin());
    check(selectDevice(devices, "", "old").status == SelectionStatus::NotFound, "Do not auto-select unrelated BLE");
    check(selectDevice({}, "", "").status == SelectionStatus::NotFound, "Empty scan");
}
} // namespace

int main()
{
    try {
        decoderTests();
        backoffTests();
        lifecycleTests();
        selectionTests();
        std::cout << "PASS: decoder, backoff, lifecycle, and device selection tests\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
