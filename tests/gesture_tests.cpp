#include "core/ScrollMotion.h"
#include "core/TopButtonGesture.h"
#include <iostream>
#include <stdexcept>

namespace {
void check(bool result, const char* message)
{
    if (!result) throw std::runtime_error(message);
}

void singleClickAndDrag()
{
    using namespace pokepointer;
    TopButtonGesture gesture;
    check(!gesture.update(true, 0).leftHeld, "First down is deferred briefly");
    check(!gesture.update(false, 90).leftHeld, "Short first release is pending");
    check(gesture.advance(409).clicks == 0, "Single click waits for double-tap window");
    check(gesture.advance(411).clicks == 1, "Single click occurs once after window");
    check(gesture.advance(1000).clicks == 0, "Single click is not repeated");
    check(gesture.update(true, 1100).clicks == 0, "New first press");
    check(!gesture.advance(1269).leftHeld, "Drag delay not yet reached");
    check(gesture.advance(1270).leftHeld, "Ordinary long press starts dragging");
    check(gesture.advance(1800).leftHeld, "Drag remains held");
    const auto released = gesture.update(false, 1850);
    check(!released.leftHeld && released.clicks == 0, "Releasing drag releases left, no extra click");
}

void doubleClickAndScroll()
{
    using namespace pokepointer;
    TopButtonGesture gesture;
    (void)gesture.update(true, 0);
    (void)gesture.update(false, 70);
    auto result = gesture.update(true, 180);
    check(result.suppressPointer && !result.leftHeld && result.clicks == 0,
          "Second press suppresses pointer and mouse click");
    check(!gesture.advance(479).scrollActive, "Scroll must not activate too early");
    result = gesture.update(false, 479);
    check(result.clicks == 2 && !result.leftHeld && !result.scrollActive,
          "Short second press resolves to double click");

    (void)gesture.update(true, 1000);
    (void)gesture.update(false, 1060);
    (void)gesture.update(true, 1180);
    result = gesture.advance(1479);
    check(!result.scrollActive && result.suppressPointer, "Scroll threshold not yet reached");
    result = gesture.advance(1480);
    check(result.scrollActive && result.suppressPointer && !result.leftHeld && result.clicks == 0,
          "Holding second press enters scroll without click or drag");
    check(gesture.advance(2000).scrollActive, "Scroll remains active while held");
    result = gesture.update(false, 2010);
    check(!result.scrollActive && !result.leftHeld && result.clicks == 0,
          "Releasing second press exits scroll without injected click");
    check(!gesture.timerNeeded(), "Idle gesture needs no timer");
}

void boundariesAndSafety()
{
    using namespace pokepointer;
    TopButtonGesture gesture;
    (void)gesture.update(true, 0);
    (void)gesture.update(false, 40);
    auto result = gesture.update(true, 360);
    check(result.suppressPointer, "Second press exactly at tap-window limit is accepted");
    result = gesture.update(false, 400);
    check(result.clicks == 2, "Double-click accepted on boundary");
    (void)gesture.update(true, 1000);
    (void)gesture.update(false, 1040);
    result = gesture.update(true, 1361);
    check(result.clicks == 1 && !result.suppressPointer,
          "Late second press emits pending single click and starts new press");
    gesture.reset();
    check(gesture.advance(3000).clicks == 0 && !gesture.timerNeeded(),
          "Reset cancels pending clicks and held inputs");
    (void)gesture.update(true, 4000);
    (void)gesture.update(false, 4040);
    gesture.reset();
    check(gesture.advance(9000).clicks == 0, "Disconnect must discard pending click");
}

void wheelMotion()
{
    using namespace pokepointer;
    ScrollMotion scroll;
    scroll.setInput({118, 114, false, false}, 114);
    check(scroll.step(1.0) == 0, "Neutral wheel input must be silent");
    scroll.setInput({118, 113, false, false}, 114);
    check(scroll.step(1.0) == 0, "Joystick jitter stays in wheel deadzone");
    scroll.setInput({118, 41, false, false}, 114);
    int upwards = 0;
    for (int i = 0; i < 100; ++i) upwards += scroll.step(0.016);
    check(upwards > 0 && upwards % 120 == 0, "Up joystick scrolls upwards in whole notches");
    scroll.reset();
    scroll.setInput({118, 180, false, false}, 114);
    int downwards = 0;
    for (int i = 0; i < 100; ++i) downwards += scroll.step(0.016);
    check(downwards < 0 && downwards % 120 == 0, "Down joystick scrolls downwards");
    scroll.reset();
    check(scroll.step(0.05) == 0, "Reset prevents scrolling with stale input");
    scroll.setInput({118, 41, false, false}, 114);
    check(scroll.step(-1.0) == 0, "Negative timestep clamped");
}
} // namespace

int main()
{
    try {
        singleClickAndDrag();
        doubleClickAndScroll();
        boundariesAndSafety();
        wheelMotion();
        std::cout << "PASS: single/double-click, drag, double-tap-hold scroll, wheel direction, and reset\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
