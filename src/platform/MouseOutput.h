#pragma once

namespace pokepointer {
// Phase 2 contract only: no backend is instantiated by the Phase 1 application.
class MouseOutput {
public:
    virtual ~MouseOutput() = default;
    virtual void moveRelative(int dx, int dy) = 0;
    // Levels, not click pulses: repeated true values must preserve a held button.
    virtual void setButtons(bool leftHeld, bool rightHeld) = 0;
    // Future backends must also release held buttons on destruction.
    virtual void releaseAll() noexcept = 0;
};
} // namespace pokepointer
