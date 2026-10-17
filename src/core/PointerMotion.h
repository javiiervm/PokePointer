#pragma once

#include "core/InputState.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

namespace pokepointer {
// Pure input-to-relative-motion mapping. No Qt or platform dependencies.
class PointerMotion final {
public:
    static constexpr std::size_t calibrationSamples = 12;

    void reset() noexcept
    {
        calibrated_ = false;
        samples_ = 0;
        sumX_ = 0;
        sumY_ = 0;
        first_.reset();
        current_.reset();
        fractionalX_ = 0.0;
        fractionalY_ = 0.0;
    }

    // Call with successive reports while the joystick is untouched.
    // Returns true only when calibration finishes on this report.
    [[nodiscard]] bool calibrate(const InputState& input) noexcept
    {
        if (calibrated_) return false;
        if (input.topPressed || input.stickPressed) {
            clearSamples();
            return false;
        }
        if (first_ && (std::abs(int(input.joystickX) - int(first_->joystickX)) > 5
                    || std::abs(int(input.joystickY) - int(first_->joystickY)) > 5)) {
            clearSamples();
        }
        if (!first_) first_ = input;
        sumX_ += input.joystickX;
        sumY_ += input.joystickY;
        ++samples_;
        if (samples_ != calibrationSamples) return false;
        centerX_ = double(sumX_) / double(samples_);
        centerY_ = double(sumY_) / double(samples_);
        calibrated_ = true;
        return true;
    }

    [[nodiscard]] bool calibrated() const noexcept { return calibrated_; }
    [[nodiscard]] double centerX() const noexcept { return centerX_; }
    [[nodiscard]] double centerY() const noexcept { return centerY_; }

    void setInput(const InputState& input) noexcept
    {
        if (calibrated_) current_ = input;
    }

    void stopMotion() noexcept
    {
        current_.reset();
        fractionalX_ = 0.0;
        fractionalY_ = 0.0;
    }

    [[nodiscard]] bool moving() const noexcept
    {
        const auto [vx, vy] = motionVector();
        return vx != 0.0 || vy != 0.0;
    }

    // Called by the application's 16 ms timer; preserves subpixel motion.
    [[nodiscard]] std::pair<int, int> step(double elapsedSeconds) noexcept
    {
        const auto [vx, vy] = motionVector();
        const double dt = std::clamp(elapsedSeconds, 0.0, 0.05);
        fractionalX_ += vx * dt;
        fractionalY_ += vy * dt;
        const int dx = static_cast<int>(std::trunc(fractionalX_));
        const int dy = static_cast<int>(std::trunc(fractionalY_));
        fractionalX_ -= dx;
        fractionalY_ -= dy;
        return {dx, dy};
    }

private:
    void clearSamples() noexcept
    {
        samples_ = 0;
        sumX_ = 0;
        sumY_ = 0;
        first_.reset();
    }

    [[nodiscard]] std::pair<double, double> motionVector() const noexcept
    {
        if (!calibrated_ || !current_) return {0.0, 0.0};
        // Measured 2026-10-17: center ~118/114 and extremes ~42..191 / 41..180.
        // The radial deadzone also suppresses the observed one-count idle jitter.
        const double x = (double(current_->joystickX) - centerX_) / 75.0;
        const double y = (double(current_->joystickY) - centerY_) / 70.0;
        const double magnitude = std::hypot(x, y);
        constexpr double deadzone = 0.14;
        if (magnitude <= deadzone) return {0.0, 0.0};
        const double strength = std::clamp((magnitude - deadzone) / (1.0 - deadzone), 0.0, 1.0);
        const double speed = 900.0 * std::pow(strength, 1.6); // pixels/second
        return {x / magnitude * speed, y / magnitude * speed};
    }

    bool calibrated_{};
    std::size_t samples_{};
    unsigned int sumX_{};
    unsigned int sumY_{};
    double centerX_{};
    double centerY_{};
    std::optional<InputState> first_;
    std::optional<InputState> current_;
    double fractionalX_{};
    double fractionalY_{};
};
} // namespace pokepointer
