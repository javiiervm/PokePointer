#pragma once

#include <cstdint>

namespace pokepointer {
// Coarse, uncalibrated device values. Axis orientation/center is not assumed.
struct InputState {
    std::uint8_t joystickX{};
    std::uint8_t joystickY{};
    bool topPressed{};
    bool stickPressed{};
    bool operator==(const InputState&) const = default;
};
} // namespace pokepointer
