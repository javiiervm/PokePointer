#pragma once

#include <cstdint>

namespace pokepointer {

struct TopGestureEffect {
    bool leftHeld{};
    bool scrollActive{};
    bool suppressPointer{};
    unsigned clicks{};
};

// Resolves a short tap, ordinary held drag, double-click, and double-tap-hold
// without injecting a left click while the scroll gesture is being recognized.
class TopButtonGesture final {
public:
    static constexpr std::int64_t dragHoldMs = 170;
    static constexpr std::int64_t secondTapWindowMs = 320;
    static constexpr std::int64_t scrollHoldMs = 300;

    void reset() noexcept
    {
        phase_ = Phase::Ready;
        physicalPressed_ = false;
        phaseSinceMs_ = 0;
    }

    [[nodiscard]] TopGestureEffect update(bool pressed, std::int64_t nowMs) noexcept
    {
        unsigned clicks = advancePhase(nowMs);
        if (pressed != physicalPressed_) {
            physicalPressed_ = pressed;
            if (pressed) {
                if (phase_ == Phase::PendingSingle) {
                    phase_ = Phase::SecondPressed;
                } else if (phase_ == Phase::Ready) {
                    phase_ = Phase::FirstPressed;
                }
                phaseSinceMs_ = nowMs;
            } else {
                switch (phase_) {
                case Phase::FirstPressed:
                    phase_ = Phase::PendingSingle;
                    phaseSinceMs_ = nowMs;
                    break;
                case Phase::SecondPressed:
                    clicks += 2;
                    phase_ = Phase::Ready;
                    break;
                case Phase::Dragging:
                case Phase::Scrolling:
                    phase_ = Phase::Ready;
                    break;
                case Phase::Ready:
                case Phase::PendingSingle:
                    break;
                }
            }
        }
        return effect(clicks);
    }

    [[nodiscard]] TopGestureEffect advance(std::int64_t nowMs) noexcept
    {
        return effect(advancePhase(nowMs));
    }

    [[nodiscard]] bool timerNeeded() const noexcept
    {
        return phase_ == Phase::FirstPressed || phase_ == Phase::PendingSingle
            || phase_ == Phase::SecondPressed || phase_ == Phase::Scrolling;
    }

    [[nodiscard]] bool suppressPointer() const noexcept
    {
        return phase_ == Phase::SecondPressed || phase_ == Phase::Scrolling;
    }

private:
    enum class Phase { Ready, FirstPressed, PendingSingle, SecondPressed, Dragging, Scrolling };

    [[nodiscard]] unsigned advancePhase(std::int64_t nowMs) noexcept
    {
        const auto elapsed = nowMs >= phaseSinceMs_ ? nowMs - phaseSinceMs_ : 0;
        switch (phase_) {
        case Phase::FirstPressed:
            if (elapsed >= dragHoldMs) phase_ = Phase::Dragging;
            break;
        case Phase::PendingSingle:
            if (elapsed > secondTapWindowMs) {
                phase_ = Phase::Ready;
                return 1;
            }
            break;
        case Phase::SecondPressed:
            if (elapsed >= scrollHoldMs) phase_ = Phase::Scrolling;
            break;
        case Phase::Ready:
        case Phase::Dragging:
        case Phase::Scrolling:
            break;
        }
        return 0;
    }

    [[nodiscard]] TopGestureEffect effect(unsigned clicks) const noexcept
    {
        return {phase_ == Phase::Dragging, phase_ == Phase::Scrolling,
                suppressPointer(), clicks};
    }

    Phase phase_{Phase::Ready};
    bool physicalPressed_{};
    std::int64_t phaseSinceMs_{};
};
} // namespace pokepointer
