#include "core/PointerMotion.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}

int main()
{
    try {
        using namespace pokepointer;
        PointerMotion pointer;
        const InputState idle{118, 114, false, false};
        for (std::size_t i = 1; i < PointerMotion::calibrationSamples; ++i)
            check(!pointer.calibrate(idle), "Calibration must wait for twelve samples");
        check(pointer.calibrate(idle), "Stable samples must calibrate");
        check(pointer.calibrated(), "Calibration state");
        check(pointer.centerX() == 118.0 && pointer.centerY() == 114.0, "Calibrated center");
        pointer.setInput(idle);
        for (int i = 0; i < 100; ++i)
            check(pointer.step(0.016) == std::pair{0, 0}, "Neutral must not move");
        pointer.setInput({119, 113, false, false});
        check(!pointer.moving(), "One-count jitter stays in deadzone");
        check(pointer.step(0.016) == std::pair{0, 0}, "Jitter must not move cursor");

        pointer.setInput({191, 114, false, false});
        check(pointer.moving(), "Right deflection moves");
        auto [rightX, rightY] = pointer.step(0.05);
        check(rightX > 0 && rightY == 0, "Right deflection direction");
        pointer.stopMotion();
        check(!pointer.moving(), "Stopping disables movement");

        pointer.setInput({42, 114, false, false});
        auto [leftX, leftY] = pointer.step(0.05);
        check(leftX < 0 && leftY == 0, "Left deflection direction");
        pointer.stopMotion();
        pointer.setInput({118, 41, false, false});
        auto [upX, upY] = pointer.step(0.05);
        check(upX == 0 && upY < 0, "Up deflection direction");
        pointer.stopMotion();
        pointer.setInput({118, 180, false, false});
        auto [downX, downY] = pointer.step(0.05);
        check(downX == 0 && downY > 0, "Down deflection direction");
        pointer.stopMotion();

        pointer.reset();
        check(!pointer.calibrated(), "Reset must forget calibration");
        for (int i = 0; i < 6; ++i)
            (void)pointer.calibrate(idle);
        check(!pointer.calibrate({118, 114, true, false}), "Pressed button interrupts calibration");
        for (int i = 0; i < 6; ++i)
            (void)pointer.calibrate(idle);
        check(!pointer.calibrate({180, 114, false, false}), "Moving joystick interrupts calibration");
        for (std::size_t i = 1; i < PointerMotion::calibrationSamples - 1; ++i)
            check(!pointer.calibrate({180, 114, false, false}), "New pose needs full sample window");
        check(pointer.calibrate({180, 114, false, false}), "Stable pose can eventually calibrate");
        pointer.setInput({180, 114, false, false});
        check(pointer.step(1.0) == std::pair{0, 0}, "Calibrated center must not move");
        pointer.reset();
        check(!pointer.moving() && pointer.step(0.05) == std::pair{0, 0},
              "Reset must prevent movement after disconnect");
        std::cout << "PASS: pointer calibration, deadzone, directions, and reset\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
