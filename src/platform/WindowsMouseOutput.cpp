#include "platform/WindowsMouseOutput.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <array>
#include <cstdio>

namespace pokepointer {
namespace {
INPUT mouseEvent(DWORD flags, LONG x = 0, LONG y = 0)
{
    INPUT event{};
    event.type = INPUT_MOUSE;
    event.mi.dx = x;
    event.mi.dy = y;
    event.mi.dwFlags = flags;
    return event;
}
} // namespace

void WindowsMouseOutput::reportFailure() noexcept
{
    if (warned_) return;
    warned_ = true;
    std::fprintf(stderr, "Windows SendInput failed (GetLastError=%lu); check input permissions.\n",
                 static_cast<unsigned long>(GetLastError()));
}

WindowsMouseOutput::~WindowsMouseOutput()
{
    releaseAll();
}

void WindowsMouseOutput::moveRelative(int dx, int dy)
{
    if (dx == 0 && dy == 0) return;
    auto event = mouseEvent(MOUSEEVENTF_MOVE, static_cast<LONG>(dx), static_cast<LONG>(dy));
    if (SendInput(1, &event, static_cast<int>(sizeof(INPUT))) != 1)
        reportFailure();
    else
        warned_ = false;
}

void WindowsMouseOutput::setButtons(bool leftHeld, bool rightHeld)
{
    std::array<INPUT, 2> events{};
    UINT count = 0;
    if (leftHeld_ != leftHeld)
        events[count++] = mouseEvent(leftHeld ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
    if (rightHeld_ != rightHeld)
        events[count++] = mouseEvent(rightHeld ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP);
    if (count == 0) return;
    if (SendInput(count, events.data(), static_cast<int>(sizeof(INPUT))) == count) {
        leftHeld_ = leftHeld;
        rightHeld_ = rightHeld;
        warned_ = false;
    } else {
        reportFailure();
    }
}

void WindowsMouseOutput::releaseAll() noexcept
{
    setButtons(false, false);
}
} // namespace pokepointer
