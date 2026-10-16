#include "app/Application.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>
#include <span>
#include <utility>

namespace pokepointer {
Application::Application(BleOptions options, bool debugRaw, DeviceStore& store, QObject* parent)
    : QObject(parent), store_(store), session_(std::move(options)), debugRaw_(debugRaw)
{
    connect(&session_, &BleSession::message, this, &Application::print);
    connect(&session_, &BleSession::stateChanged, this, [this](ConnectionState state) {
        print("State: " + QString::fromLatin1(stateName(state)));
    });
    connect(&session_, &BleSession::inputNotification, this, &Application::onInput);
    connect(&session_, &BleSession::inputInvalidated, this, [this] {
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
    connect(&consoleStop_, &ConsoleStop::requested, this, [this] { session_.stop(); });
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [this] { session_.stop(); });
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
    print("PokePointer 0.1: BLE input diagnostics. Native mouse output is not implemented.");
    print("Device settings: " + store_.fileName());
    session_.start();
}

void Application::print(const QString& text) const
{
    QTextStream output(stdout);
    output << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) << " " << text << Qt::endl;
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
        // Do not retain a held-button interpretation across unknown input formats.
        previousInput_.reset();
        return;
    }
    previousError_.reset();
    if (previousInput_ == decoded.state) return;
    previousInput_ = decoded.state;
    const auto& input = *decoded.state;
    print(QString("Input raw-x=%1 raw-y=%2 top=%3 stick=%4")
              .arg(input.joystickX).arg(input.joystickY)
              .arg(input.topPressed ? "held" : "released", input.stickPressed ? "held" : "released"));
}
} // namespace pokepointer
