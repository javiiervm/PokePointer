#include "app/DeviceStore.h"
#include "ble/BleSession.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    try {
        using namespace pokepointer;
        QTemporaryDir temporary;
        check(temporary.isValid(), "Temporary settings directory");
        const auto file = temporary.filePath("device.ini");
        DeviceStore store(file);
        check(store.load().isEmpty(), "No selection on first launch");
        check(store.save("aa:bb:cc:dd:ee:ff"), "Save selection");
        check(DeviceStore(file).load() == "aa:bb:cc:dd:ee:ff", "Persistence across store instances");
        check(store.forget() && DeviceStore(file).load().isEmpty(), "Forget persisted identity");
        check(store.forget(), "Forgetting is idempotent");
        check(canonicalIdentifier(" AA:BB:CC:DD:EE:FF ") == "aa:bb:cc:dd:ee:ff", "Address normalization");
        check(canonicalIdentifier("{6675E16C-F36D-4567-BB55-6B51E27A23E5}")
              == "6675e16c-f36d-4567-bb55-6b51e27a23e5", "UUID normalization");
        check(canonicalIdentifier("not-an-identifier").isEmpty(), "Reject invalid identity");

        // No radio access: stop before start, then ensure no restart or duplicate completion.
        BleSession session({});
        int completions = 0;
        int invalidations = 0;
        QObject::connect(&session, &BleSession::finished, &app, [&](int code) {
            check(code == 0, "Normal shutdown exit code");
            ++completions;
        });
        QObject::connect(&session, &BleSession::inputInvalidated, &app, [&] { ++invalidations; });
        session.stop();
        session.stop();
        session.start();
        QCoreApplication::processEvents();
        check(session.state() == ConnectionState::Stopped, "Stop before start remains stopped");
        check(completions == 1 && invalidations == 1, "Shutdown is idempotent");
        std::cout << "PASS: settings, identifier normalization, and offline session shutdown\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
