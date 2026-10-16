#include "ble/BleSession.h"
#include "core/DeviceSelection.h"
#include "protocol/InputDecoder.h"
#include <QBluetoothAddress>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QStringList>
#include <utility>
#include <vector>

namespace pokepointer {
namespace {
QBluetoothUuid inputServiceId()
{
    return QBluetoothUuid(QString::fromLatin1(inputServiceUuid.data(),
                                             static_cast<qsizetype>(inputServiceUuid.size())));
}
QBluetoothUuid inputCharacteristicId()
{
    return QBluetoothUuid(QString::fromLatin1(inputCharacteristicUuid.data(),
                                             static_cast<qsizetype>(inputCharacteristicUuid.size())));
}
QString propertiesText(QLowEnergyCharacteristic::PropertyTypes properties)
{
    QStringList names;
    using C = QLowEnergyCharacteristic;
    if (properties.testFlag(C::Read)) names << "read";
    if (properties.testFlag(C::Write)) names << "write";
    if (properties.testFlag(C::WriteNoResponse)) names << "write-without-response";
    if (properties.testFlag(C::Notify)) names << "notify";
    if (properties.testFlag(C::Indicate)) names << "indicate";
    if (properties.testFlag(C::Broadcasting)) names << "broadcast";
    if (properties.testFlag(C::WriteSigned)) names << "signed-write";
    if (properties.testFlag(C::ExtendedProperty)) names << "extended";
    return names.join(',');
}
} // namespace

QString canonicalIdentifier(const QString& identifier)
{
    const QBluetoothAddress address(identifier.trimmed());
    if (!address.isNull())
        return address.toString().toLower();
    const QBluetoothUuid uuid(identifier.trimmed());
    if (!uuid.isNull())
        return uuid.toString(QUuid::WithoutBraces).toLower();
    return {};
}

QString deviceIdentifier(const QBluetoothDeviceInfo& device)
{
    if (!device.address().isNull())
        return device.address().toString().toLower();
    if (!device.deviceUuid().isNull())
        return device.deviceUuid().toString(QUuid::WithoutBraces).toLower();
    return {};
}

BleSession::BleSession(BleOptions options, QObject* parent)
    : QObject(parent), options_(std::move(options))
{
    for (auto* timer : {&retry_, &deadline_, &stable_, &silence_})
        timer->setSingleShot(true);
    connect(&retry_, &QTimer::timeout, this, &BleSession::scan);
    connect(&deadline_, &QTimer::timeout, this, [this] {
        if (state() == ConnectionState::ShuttingDown) {
            emit message("Disconnect deadline reached; releasing local BLE resources.");
            finishStop();
        } else {
            fail("BLE operation timed out in " + QString::fromLatin1(stateName(state())));
        }
    });
    connect(&stable_, &QTimer::timeout, this, [this] {
        if (state() == ConnectionState::Streaming && receivedInput_)
            backoff_.reset();
    });
    connect(&silence_, &QTimer::timeout, this, [this] {
        if (state() == ConnectionState::Streaming)
            emit message("No input notification for 15 seconds. Link may be idle or asleep; "
                         "no verified keep-alive is available. Use --debug-raw and test the controls.");
    });
}

BleSession::~BleSession()
{
    // Fallback for destruction without an event-loop shutdown; children remain owned by this.
    retireObjects();
}

bool BleSession::current(quint64 generation) const
{
    return generation == generation_ && state() != ConnectionState::ShuttingDown
        && state() != ConnectionState::Stopped && state() != ConnectionState::Reconnecting;
}

void BleSession::setState(ConnectionState next)
{
    if (!lifecycle_.transition(next))
        qFatal("Invalid BLE lifecycle transition: %s -> %s", stateName(state()), stateName(next));
    emit stateChanged(next);
}

void BleSession::start()
{
    if (state() == ConnectionState::Idle)
        scan();
}

void BleSession::scan()
{
    if (state() != ConnectionState::Idle && state() != ConnectionState::Reconnecting)
        return;
    setState(ConnectionState::Scanning);
    devices_.clear();
    scanner_ = new QBluetoothDeviceDiscoveryAgent(this);
    scanner_->setLowEnergyDiscoveryTimeout(8000);
    const auto generation = generation_;
    connect(scanner_, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this,
            [this, generation](const QBluetoothDeviceInfo& device) {
        if (!current(generation) || state() != ConnectionState::Scanning
            || !device.coreConfigurations().testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
            return;
        const auto id = deviceIdentifier(device);
        if (id.isEmpty()) return;
        const bool first = !devices_.contains(id);
        devices_.insert(id, device);
        if (options_.scanOnly && first)
            emit message(QString("Device %1 name=\"%2\" RSSI=%3")
                             .arg(id, device.name()).arg(device.rssi()));
    });
    connect(scanner_, &QBluetoothDeviceDiscoveryAgent::finished, this, [this, generation] {
        if (current(generation) && state() == ConnectionState::Scanning)
            scanFinished();
    });
    connect(scanner_, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this,
            [this, generation](QBluetoothDeviceDiscoveryAgent::Error error) {
        if (!current(generation) || state() != ConnectionState::Scanning || !scanner_) return;
        const bool fatal = error == QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod
                        || error == QBluetoothDeviceDiscoveryAgent::MissingPermissionsError;
        fail("Discovery failed: " + scanner_->errorString(), fatal);
    });
    deadline_.start(12000);
    scanner_->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BleSession::scanFinished()
{
    deadline_.stop();
    // Include any updated advertisement information returned by the backend.
    for (const auto& device : scanner_->discoveredDevices()) {
        const auto id = deviceIdentifier(device);
        if (!id.isEmpty() && device.coreConfigurations().testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
            devices_.insert(id, device);
    }
    scanner_->disconnect(this);
    scanner_->deleteLater();
    scanner_ = nullptr;
    if (options_.scanOnly) {
        emit message(QString("Scan complete: %1 BLE device(s).").arg(devices_.size()));
        stop();
        return;
    }
    std::vector<DeviceCandidate> candidates;
    for (auto it = devices_.cbegin(); it != devices_.cend(); ++it) {
        const auto& device = it.value();
        const bool likely = device.name().compare("Pokemon PBP", Qt::CaseInsensitive) == 0
                         || device.serviceUuids().contains(inputServiceId());
        candidates.push_back({it.key().toStdString(), likely});
    }
    const auto selection = selectDevice(candidates, options_.explicitIdentifier.toStdString(),
                                         options_.cachedIdentifier.toStdString());
    if (selection.status == SelectionStatus::Ambiguous) {
        fail("Multiple Pokeball candidates found. Run --scan, then select one with --device <identifier>.", true);
        return;
    }
    if (!selection.index) {
        fail("Selected device was not found. Wake the Pokeball and keep it nearby.");
        return;
    }
    if (selection.replacedCachedIdentity)
        emit message("Cached identity is absent; trying the only advertised Pokeball candidate.");
    const auto id = QString::fromStdString(candidates[*selection.index].identifier);
    connectDevice(devices_.value(id));
}

void BleSession::connectDevice(const QBluetoothDeviceInfo& device)
{
    setState(ConnectionState::Connecting);
    connectingIdentifier_ = deviceIdentifier(device);
    emit message("Connecting to " + connectingIdentifier_ + " (" + device.name() + ").");
    controller_ = QLowEnergyController::createCentral(device, this);
    if (!controller_) {
        fail("Could not create the BLE controller.");
        return;
    }
    const auto generation = generation_;
    connect(controller_, &QLowEnergyController::connected, this, [this, generation] {
        if (!current(generation) || state() != ConnectionState::Connecting) return;
        setState(ConnectionState::Discovering);
        deadline_.start(20000);
        controller_->discoverServices();
    });
    connect(controller_, &QLowEnergyController::disconnected, this, [this, generation] {
        if (generation != generation_) return;
        if (state() == ConnectionState::ShuttingDown)
            finishStop();
        else if (current(generation))
            fail("BLE device disconnected.");
    });
    connect(controller_, &QLowEnergyController::errorOccurred, this,
            [this, generation](QLowEnergyController::Error error) {
        if (!current(generation)) return;
        fail("Controller error: " + controller_->errorString(),
             error == QLowEnergyController::MissingPermissionsError
             || error == QLowEnergyController::AuthorizationError);
    });
    connect(controller_, &QLowEnergyController::discoveryFinished, this, [this, generation] {
        if (!current(generation) || state() != ConnectionState::Discovering) return;
        pendingServices_ = controller_->services();
        if (!options_.inspectGatt) {
            if (!pendingServices_.contains(inputServiceId())) {
                fail("Known input service is absent. Run --inspect-gatt for diagnostics.");
                return;
            }
            pendingServices_ = {inputServiceId()};
        }
        discoverNextService();
    });
    deadline_.start(20000);
    controller_->connectToDevice();
}

void BleSession::discoverNextService()
{
    if (pendingServices_.isEmpty()) {
        if (options_.inspectGatt) {
            emit message("GATT inspection complete. No notification subscriptions or proprietary writes were performed.");
            stop();
        }
        return;
    }
    const auto uuid = pendingServices_.takeFirst();
    auto* service = controller_->createServiceObject(uuid, controller_);
    if (!service) {
        fail("Could not create GATT service " + uuid.toString());
        return;
    }
    activeService_ = service;
    const auto generation = generation_;
    connect(service, &QLowEnergyService::errorOccurred, this,
            [this, generation, service](QLowEnergyService::ServiceError error) {
        if (current(generation) && activeService_ == service)
            fail(QString("GATT service %1 error %2").arg(service->serviceUuid().toString())
                     .arg(static_cast<int>(error)));
    });
    connect(service, &QLowEnergyService::stateChanged, this,
            [this, generation, service](QLowEnergyService::ServiceState serviceState) {
        if (!current(generation) || activeService_ != service) return;
        if (serviceState == QLowEnergyService::RemoteServiceDiscovered)
            serviceReady(service);
        else if (serviceState == QLowEnergyService::InvalidService)
            fail("GATT service became invalid.");
    });
    deadline_.start(15000);
    if (service->state() == QLowEnergyService::RemoteServiceDiscovered)
        serviceReady(service);
    else
        service->discoverDetails(QLowEnergyService::SkipValueDiscovery);
}

void BleSession::serviceReady(QLowEnergyService* service)
{
    if (state() != ConnectionState::Discovering) return;
    if (options_.inspectGatt) {
        emit message("Service " + service->serviceUuid().toString());
        for (const auto& characteristic : service->characteristics()) {
            emit message(QString("  Characteristic %1 properties=[%2]")
                             .arg(characteristic.uuid().toString())
                             .arg(propertiesText(characteristic.properties())));
            for (const auto& descriptor : characteristic.descriptors())
                emit message(QString("    Descriptor %1").arg(descriptor.uuid().toString()));
        }
        service->disconnect(this);
        service->deleteLater();
        activeService_ = nullptr;
        discoverNextService();
    } else {
        subscribe(service);
    }
}

void BleSession::subscribe(QLowEnergyService* service)
{
    const auto input = service->characteristic(inputCharacteristicId());
    const auto cccd = input.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!input.isValid() || !input.properties().testFlag(QLowEnergyCharacteristic::Notify) || !cccd.isValid()) {
        fail("Input characteristic, Notify property, or CCCD is missing. Run --inspect-gatt.");
        return;
    }
    setState(ConnectionState::Subscribing);
    const auto generation = generation_;
    connect(service, &QLowEnergyService::characteristicChanged, this,
            [this, generation, input](const QLowEnergyCharacteristic& characteristic, const QByteArray& value) {
        if (!current(generation) || characteristic != input) return;
        if (state() == ConnectionState::Subscribing) {
            // Some backends can deliver the first value before the CCCD acknowledgment.
            pendingInput_ = value;
        } else if (state() == ConnectionState::Streaming) {
            deliverInput(value);
        }
    });
    connect(service, &QLowEnergyService::descriptorWritten, this,
            [this, generation, cccd](const QLowEnergyDescriptor& descriptor, const QByteArray& value) {
        if (!current(generation) || state() != ConnectionState::Subscribing || descriptor != cccd) return;
        if (value != QByteArray::fromHex("0100")) {
            fail("Input notification subscription was not acknowledged as enabled.");
            return;
        }
        deadline_.stop();
        setState(ConnectionState::Streaming);
        receivedInput_ = false;
        stable_.start(30000);
        silence_.start(15000);
        options_.cachedIdentifier = connectingIdentifier_;
        emit selectedDevice(connectingIdentifier_);
        emit message("Input notifications enabled. Streaming means subscription confirmed; "
                     "physical input and idle stability still require validation.");
        if (pendingInput_) {
            const auto firstInput = std::exchange(pendingInput_, std::nullopt);
            deliverInput(*firstInput);
        }
    });
    deadline_.start(10000);
    // Standard CCCD write only. No vendor initialization or keep-alive commands.
    service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
}

void BleSession::deliverInput(const QByteArray& value)
{
    if (!receivedInput_ && !stable_.isActive())
        backoff_.reset(); // First input after the 30-second stability window also qualifies.
    receivedInput_ = true;
    silence_.start(15000);
    emit inputNotification(value);
}

void BleSession::fail(const QString& reason, bool fatal)
{
    if (state() == ConnectionState::ShuttingDown || state() == ConnectionState::Stopped
        || state() == ConnectionState::Reconnecting)
        return;
    emit message(reason);
    if (fatal || options_.scanOnly || options_.inspectGatt) {
        stop(1);
        return;
    }
    setState(ConnectionState::Reconnecting);
    retireObjects();
    emit inputInvalidated();
    const auto delay = backoff_.next();
    emit message(QString("Retrying with a fresh scan in %1 ms.").arg(delay.count()));
    retry_.start(static_cast<int>(delay.count()));
}

void BleSession::retireObjects()
{
    ++generation_;
    retry_.stop();
    deadline_.stop();
    stable_.stop();
    silence_.stop();
    activeService_ = nullptr;
    pendingInput_.reset();
    pendingServices_.clear();
    if (scanner_) {
        scanner_->disconnect(this);
        scanner_->stop();
        scanner_->deleteLater();
        scanner_ = nullptr;
    }
    if (controller_) {
        controller_->disconnect(this);
        // Service callbacks are additionally guarded by generation_.
        controller_->disconnectFromDevice();
        controller_->deleteLater();
        controller_ = nullptr;
    }
}

void BleSession::stop(int exitCode)
{
    if (state() == ConnectionState::ShuttingDown || state() == ConnectionState::Stopped) return;
    exitCode_ = exitCode;
    setState(ConnectionState::ShuttingDown);
    retry_.stop();
    deadline_.stop();
    stable_.stop();
    silence_.stop();
    if (scanner_) {
        scanner_->disconnect(this);
        scanner_->stop();
    }
    emit inputInvalidated();
    if (!controller_ || controller_->state() == QLowEnergyController::UnconnectedState) {
        finishStop();
        return;
    }
    deadline_.start(2000);
    controller_->disconnectFromDevice();
}

void BleSession::finishStop()
{
    if (state() != ConnectionState::ShuttingDown) return;
    retireObjects();
    setState(ConnectionState::Stopped);
    emit finished(exitCode_);
}
} // namespace pokepointer
