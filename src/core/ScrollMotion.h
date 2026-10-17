#pragma once

#include "core/InputState.h"
#include <algorithm>
#include <cmath>
#include <optional>

namespace pokepointer {

// Qt-free wheel mapping: one standard 120-unit notch per accumulated step.
// Negative joystick Y changes scroll upward, as on the tested controller.
class ScrollMotion final {
public:
    void setInput(const InputState& input, double centerY) noexcept
    {
        currentY_ = static_cast<double>(input.joystickY);
        centerY_ = centerY;
    }

    void reset() noexcept
    {
        currentY_.reset();
        remainder_ = 0.0;
    }

    [[nodiscard]] int step(double elapsedSeconds) noexcept
    {
        if (!currentY_) return 0;
        const double displacement = std::clamp((centerY_ - *currentY_) / 70.0, -1.0, 1.0);
        constexpr double deadzone = 0.14;
        if (std::abs(displacement) <= deadzone) return 0;
        const double strength = (std::abs(displacement) - deadzone) / (1.0 - deadzone);
        const double notchesPerSecond = 10.0 * std::pow(strength, 1.45);
        const double direction = displacement > 0.0 ? 1.0 : -1.0;
        remainder_ += direction * notchesPerSecond * std::clamp(elapsedSeconds, 0.0, 0.05);
        const int wholeNotches = static_cast<int>(std::trunc(remainder_));
        remainder_ -= wholeNotches;
        return wholeNotches * 120;
    }

private:
    std::optional<double> currentY_;
    double centerY_{};
    double remainder_{};
};
} // namespace pokepointer
