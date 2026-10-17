#pragma once

#include "app/DeviceStore.h"
#include "ble/BleSession.h"
#include "core/PointerMotion.h"
#include "core/ScrollMotion.h"
#include "core/TopButtonGesture.h"
#include "platform/ConsoleStop.h"
#include "platform/MouseOutput.h"
#include "protocol/InputDecoder.h"
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <memory>
#include <optional>

namespace pokepointer {
class Application final : public QObject {
    Q_OBJECT
public:
    Application(BleOptions options, bool debugRaw, bool mouseEnabled, DeviceStore& store,
                QObject* parent = nullptr);
    void start();
private:
    void onInput(const QByteArray& packet);
    void resetMouse();
    void applyTopGesture(const TopGestureEffect& effect);
    void refreshMotionTimer();
    void print(const QString& text) const;
    DeviceStore& store_;
    BleSession session_;
    ConsoleStop consoleStop_;
    bool debugRaw_{};
    bool mouseEnabled_{};
    std::unique_ptr<MouseOutput> mouseOutput_;
    PointerMotion pointer_;
    ScrollMotion scroll_;
    TopButtonGesture topGesture_;
    std::optional<InputState> currentMouseInput_;
    QElapsedTimer gestureClock_;
    QTimer motionTimer_;
    QTimer mouseSafetyTimer_;
    QElapsedTimer motionClock_;
    std::optional<InputState> previousInput_;
    std::optional<DecodeError> previousError_;
    QByteArray previousRaw_;
    QTimer rawSummary_;
    int rawPrinted_{};
    quint64 rawSuppressed_{};
};
} // namespace pokepointer
