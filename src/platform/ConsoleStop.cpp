#include "platform/ConsoleStop.h"
#include <QDebug>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace pokepointer {
namespace {
#ifdef Q_OS_WIN
volatile LONG stopRequested = 0;
BOOL WINAPI consoleHandler(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&stopRequested, 1);
    return TRUE;
}
bool takeStopRequest() { return InterlockedExchange(&stopRequested, 0) != 0; }
#else
volatile std::sig_atomic_t stopRequested = 0;
void consoleHandler(int) { stopRequested = 1; }
bool takeStopRequest()
{
    // The flag stays set; the timer is stopped on the first request.
    return stopRequested != 0;
}
#endif
} // namespace

ConsoleStop::ConsoleStop(QObject* parent) : QObject(parent)
{
#ifdef Q_OS_WIN
    installed_ = SetConsoleCtrlHandler(consoleHandler, TRUE) != FALSE;
    if (!installed_) qWarning("Could not install the console shutdown handler.");
#else
    stopRequested = 0;
    previousInterrupt_ = std::signal(SIGINT, consoleHandler);
    previousTerminate_ = std::signal(SIGTERM, consoleHandler);
#endif
    connect(&poll_, &QTimer::timeout, this, [this] {
        if (takeStopRequest()) {
            poll_.stop();
            emit requested();
        }
    });
    poll_.start(50);
}

ConsoleStop::~ConsoleStop()
{
#ifdef Q_OS_WIN
    if (installed_) SetConsoleCtrlHandler(consoleHandler, FALSE);
#else
    if (previousInterrupt_ != SIG_ERR) std::signal(SIGINT, previousInterrupt_);
    if (previousTerminate_ != SIG_ERR) std::signal(SIGTERM, previousTerminate_);
#endif
}
} // namespace pokepointer
