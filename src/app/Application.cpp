#include "app/Application.h"
#ifdef Q_OS_WIN
#include "platform/WindowsMouseOutput.h"
#endif
#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>
#include <span>
#include <utility>

namespace pokepointer {
Application::Application(BleOptions options, bool debugRaw, bool mouseEnabled,
                         DeviceStore& store, QObject* parent)
    : QObject(parent), store_(store), session_(std::move(options)),
      debugRaw_(debugRaw), mouseEnabled_(mouseEnabled)
{
#ifdef Q_OS_WIN
    if (mouseEnabled_) mouseOutput_ = std::make_unique<WindowsMouseOutput>();
#endif
    gestureClock_.start();
    motionTimer_.setInterval(16);
    motionTimer_.setTimerType(Qt::PreciseTimer);
    connect(&motionTimer_, &QTimer::timeout, this, [this] {
        if (!mouseOutput_ || !currentMouseInput_) return;
        const double elapsedSeconds = double(motionClock_.restart()) / 1000.0;
        const auto effect = topGesture_.advance(gestureClock_.elapsed());
        applyTopGesture(effect);
        if (effect.scrollActive) {
            pointer_.stopMotion();
            scroll_.setInput(*currentMouseInput_, pointer_.centerY());
            mouseOutput_->scrollVertical(scroll_.step(elapsedSeconds));
        } else {
            scroll_.reset();
            if (effect.suppressPointer) {
                pointer_.stopMotion();
            } else {
                pointer_.setInput(*currentMouseInput_);
                const auto [dx, dy] = pointer_.step(elapsedSeconds);
                mouseOutput_->moveRelative(dx, dy);
            }
        }
        refreshMotionTimer();
    });
    mouseSafetyTimer_.setSingleShot(true);
    connect(&mouseSafetyTimer_, &QTimer::timeout, this, [this] {
        if (!mouseOutput_ || !pointer_.calibrated()) return;
        resetMouse();
        print("Mouse input timeout (500 ms): movement stopped, buttons released, and calibration reset.");
    });
    connect(&session_, &BleSession::message, this, &Application::print);
    connect(&session_, &BleSession::stateChanged, this, [this](ConnectionState state) {
        print("State: " + QString::fromLatin1(stateName(state)));
    });
    connect(&session_, &BleSession::inputNotification, this, &Application::onInput);
    connect(&session_, &BleSession::inputInvalidated, this, [this] {
        resetMouse();
        if (previousInput_) print("Input state unavailable; previous button levels are no longer valid.");
        previousInput_.reset();
        previousError_.reset();
        previousRaw_.clear();
    });
    connect(&session_, &BleSession::selectedDevice, this, [this](const QString& identifier) {
        if (!store_.save(identifier))
            print("Could not persist device selection at " + store_.fileName());
    });
    connect(&session_, &BleSession::finished, this, [](int code) { QCoreApplication::exit(code); });
    connect(&consoleStop_, &ConsoleStop::requested, this, [this] {
        resetMouse();
        session_.stop();
    });
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [this] {
        resetMouse();
        session_.stop();
    });
    connect(&rawSummary_, &QTimer::timeout, this, [this] {
        if (rawSuppressed_ != 0)
            print(QString("Raw diagnostics: %1 duplicate/rate-limited packet(s) suppressed this second.")
                      .arg(rawSuppressed_));
        rawPrinted_ = 0;
        rawSuppressed_ = 0;
    });
    if (debugRaw_) rawSummary_.start(1000);
}

void Application::start()
{
    print(mouseEnabled_ ? "PokePointer 0.3: Windows mouse mode. Top=left/drag; double-tap-hold top=scroll; stick=right."
                        : "PokePointer 0.3: BLE input diagnostics; native mouse output disabled.");
    if (mouseEnabled_)
        print("Keep the joystick untouched during calibration. Double-tap top and hold the second press 300 ms to scroll. Ctrl+C to stop.");
    print("Device settings: " + store_.fileName());
    session_.start();
}

void Application::print(const QString& text) const
{
    QTextStream output(stdout);
    output << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) << " " << text << Qt::endl;
}

void Application::resetMouse()
{
    motionTimer_.stop();
    mouseSafetyTimer_.stop();
    topGesture_.reset();
    scroll_.reset();
    currentMouseInput_.reset();
    pointer_.reset();
    if (mouseOutput_) mouseOutput_->releaseAll();
}

void Application::applyTopGesture(const TopGestureEffect& effect)
{
    if (!mouseOutput_) return;
    const bool rightHeld = currentMouseInput_.has_value() && currentMouseInput_->stickPressed;
    mouseOutput_->setButtons(effect.leftHeld, rightHeld);
    // Deferred taps are emitted only once the controller rules out a scroll gesture.
    // Consecutive complete click pulses preserve ordinary Windows double-click behavior.
    for (unsigned i = 0; i < effect.clicks; ++i) {
        mouseOutput_->setButtons(true, rightHeld);
        mouseOutput_->setButtons(false, rightHeld);
    }
}

void Application::refreshMotionTimer()
{
    if (!mouseOutput_ || !pointer_.calibrated()) return;
    const bool needsTicks = topGesture_.timerNeeded()
        || (!topGesture_.suppressPointer() && pointer_.moving());
    if (needsTicks && !motionTimer_.isActive()) {
        motionClock_.start();
        motionTimer_.start();
    } else if (!needsTicks) {
        motionTimer_.stop();
        pointer_.stopMotion();
    }
}

void Application::onInput(const QByteArray& packet)
{
    if (debugRaw_) {
        if (packet == previousRaw_ || rawPrinted_ >= 10) {
            ++rawSuppressed_;
        } else {
            // Bounded preview for unexpected long reports; length remains visible.
            print(QString("Raw input %1 length=%2 hex=%3%4")
                      .arg(QString::fromLatin1(inputCharacteristicUuid.data(),
                                              static_cast<qsizetype>(inputCharacteristicUuid.size())))
                      .arg(packet.size()).arg(QString::fromLatin1(packet.left(512).toHex(' ')),
                                              packet.size() > 512 ? " [truncated]" : ""));
            previousRaw_ = packet;
            ++rawPrinted_;
        }
    }
    const auto decoded = decodeInput(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t*>(packet.constData()), static_cast<std::size_t>(packet.size())));
    if (!decoded.state) {
        if (previousError_ != decoded.error)
            print("Input not decoded: " + QString::fromLatin1(decodeErrorText(decoded.error))
                  + ". Use --debug-raw for packet diagnostics.");
        previousError_ = decoded.error;
        previousInput_.reset();
        resetMouse();
        return;
    }
    previousError_.reset();
    const auto& input = *decoded.state;
    if (mouseOutput_) {
        mouseSafetyTimer_.start(500);
        if (!pointer_.calibrated() && pointer_.calibrate(input)) {
            print(QString("Mouse calibrated: center-x=%1 center-y=%2; pointer enabled.")
                      .arg(pointer_.centerX(), 0, 'f', 1)
                      .arg(pointer_.centerY(), 0, 'f', 1));
        }
        if (pointer_.calibrated()) {
            currentMouseInput_ = input;
            const auto effect = topGesture_.update(input.topPressed, gestureClock_.elapsed());
            applyTopGesture(effect);
            if (effect.suppressPointer) {
                pointer_.stopMotion();
            } else {
                pointer_.setInput(input);
            }
            if (!effect.scrollActive) scroll_.reset();
            refreshMotionTimer();
        }
    }
    if (previousInput_ == decoded.state) return;
    previousInput_ = decoded.state;
    if (!mouseEnabled_ || debugRaw_) {
        print(QString("Input raw-x=%1 raw-y=%2 top=%3 stick=%4")
                  .arg(input.joystickX).arg(input.joystickY)
                  .arg(input.topPressed ? "held" : "released", input.stickPressed ? "held" : "released"));
    }
}
} // namespace pokepointer
