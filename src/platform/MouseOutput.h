#pragma once

namespace pokepointer {
class MouseOutput {
public:
    virtual ~MouseOutput() = default;
    virtual void moveRelative(int dx, int dy) = 0;
    // Levels, not click pulses: repeated true values must preserve a held button.
    virtual void setButtons(bool leftHeld, bool rightHeld) = 0;
    // Positive delta scrolls up; standard wheel notches are multiples of 120.
    virtual void scrollVertical(int wheelDelta) = 0;
    // Every backend must release held buttons on destruction and invalid input.
    virtual void releaseAll() noexcept = 0;
};
} // namespace pokepointer
