#pragma once

#include "core/Backoff.h"
#include "core/ConnectionState.h"
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <optional>

namespace pokepointer {
struct BleOptions {
    bool scanOnly{};
    bool inspectGatt{};
    QString explicitIdentifier;
    QString cachedIdentifier;
};

[[nodiscard]] QString canonicalIdentifier(const QString& identifier);
[[nodiscard]] QString deviceIdentifier(const QBluetoothDeviceInfo& device);

class BleSession final : public QObject {
    Q_OBJECT
public:
    explicit BleSession(BleOptions options, QObject* parent = nullptr);
    ~BleSession() override;
    void start();
    void stop(int exitCode = 0);
    [[nodiscard]] ConnectionState state() const { return lifecycle_.state(); }
signals:
    void message(const QString& text);
    void stateChanged(pokepointer::ConnectionState state);
    void inputNotification(const QByteArray& value);
    void inputInvalidated();
    void selectedDevice(const QString& identifier);
    void finished(int exitCode);
private:
    void setState(ConnectionState state);
    void scan();
    void scanFinished();
    void connectDevice(const QBluetoothDeviceInfo& device);
    void discoverNextService();
    void serviceReady(QLowEnergyService* service);
    void subscribe(QLowEnergyService* service);
    void deliverInput(const QByteArray& value);
    void fail(const QString& reason, bool fatal = false);
    void retireObjects();
    void finishStop();
    bool current(quint64 generation) const;

    BleOptions options_;
    Lifecycle lifecycle_;
    Backoff backoff_;
    QTimer retry_;
    QTimer deadline_;
    QTimer stable_;
    QTimer silence_;
    QPointer<QBluetoothDeviceDiscoveryAgent> scanner_;
    QPointer<QLowEnergyController> controller_;
    QPointer<QLowEnergyService> activeService_;
    QMap<QString, QBluetoothDeviceInfo> devices_;
    QList<QBluetoothUuid> pendingServices_;
    QString connectingIdentifier_;
    quint64 generation_{};
    bool receivedInput_{};
    std::optional<QByteArray> pendingInput_;
    int exitCode_{};
};
} // namespace pokepointer
