#include "platform/ConsoleStop.h"
#include <QCoreApplication>
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
// The console handler runs on a Windows-created thread. The Qt event loop
// performs the actual shutdown, then signals this event before it quits.
// Keep this process-lifetime handle valid even while a close handler is waiting.
HANDLE shutdownComplete = CreateEventW(nullptr, TRUE, FALSE, nullptr);

BOOL WINAPI consoleHandler(DWORD event)
{
    switch (event) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        InterlockedExchange(&stopRequested, 1);
        return TRUE;
    case CTRL_CLOSE_EVENT:
        InterlockedExchange(&stopRequested, 1);
        // Windows normally allows about five seconds for a console-close
        // handler. Leave time for the OS to finish closing the console.
        if (shutdownComplete)
            WaitForSingleObject(shutdownComplete, 4000);
        return TRUE;
    default:
        return FALSE;
    }
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
    InterlockedExchange(&stopRequested, 0);
    if (!shutdownComplete) qWarning("Could not create the console shutdown event.");
    installed_ = SetConsoleCtrlHandler(consoleHandler, TRUE) != FALSE;
    if (!installed_) qWarning("Could not install the console shutdown handler.");
    // aboutToQuit runs on the Qt thread after Application has requested BLE
    // shutdown and released held mouse buttons.
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [] {
        if (shutdownComplete) SetEvent(shutdownComplete);
    });
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
    // Deliberately leave shutdownComplete alive until process termination:
    // a concurrent CTRL_CLOSE_EVENT handler may still be waiting on it.
#else
    if (previousInterrupt_ != SIG_ERR) std::signal(SIGINT, previousInterrupt_);
    if (previousTerminate_ != SIG_ERR) std::signal(SIGTERM, previousTerminate_);
#endif
}
} // namespace pokepointer
