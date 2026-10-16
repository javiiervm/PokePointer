#pragma once

#include "app/DeviceStore.h"
#include "ble/BleSession.h"
#include "platform/ConsoleStop.h"
#include "protocol/InputDecoder.h"
#include <QObject>
#include <optional>

namespace pokepointer {
class Application final : public QObject {
    Q_OBJECT
public:
    Application(BleOptions options, bool debugRaw, DeviceStore& store, QObject* parent = nullptr);
    void start();
private:
    void onInput(const QByteArray& packet);
    void print(const QString& text) const;
    DeviceStore& store_;
    BleSession session_;
    ConsoleStop consoleStop_;
    bool debugRaw_{};
    std::optional<InputState> previousInput_;
    std::optional<DecodeError> previousError_;
    QByteArray previousRaw_;
    QTimer rawSummary_;
    int rawPrinted_{};
    quint64 rawSuppressed_{};
};
} // namespace pokepointer
