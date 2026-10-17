#pragma once

#include "platform/MouseOutput.h"

namespace pokepointer {
class WindowsMouseOutput final : public MouseOutput {
public:
    ~WindowsMouseOutput() override;
    void moveRelative(int dx, int dy) override;
    void setButtons(bool leftHeld, bool rightHeld) override;
    void scrollVertical(int wheelDelta) override;
    void releaseAll() noexcept override;

private:
    bool leftHeld_{};
    bool rightHeld_{};
    bool warned_{};
    void reportFailure() noexcept;
};
} // namespace pokepointer
