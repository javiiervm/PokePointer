#include "app/Application.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("PokePointer");
    QCoreApplication::setApplicationVersion("0.3.0");
    QCoreApplication::setOrganizationName("PokePointer");
    QCommandLineParser parser;
    parser.setApplicationDescription("Pokeball Plus BLE diagnostics and opt-in Windows mouse control.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"scan", "List BLE advertisements for 8 seconds, then exit without connecting."});
    parser.addOption({"inspect-gatt", "Connect, list GATT services/characteristics/descriptors, then exit."});
    parser.addOption({"debug-raw", "Show input notification hex previews (at most 10 per second)."});
    parser.addOption({"mouse", "Enable Windows mouse control (top=left/drag, double-tap-hold top=scroll, stick=right)."});
    parser.addOption({"device", "Select an address or device UUID printed by --scan.", "identifier"});
    parser.addOption({"forget-device", "Clear the saved selection; when used alone, exit."});
    parser.process(app);
    const bool scan = parser.isSet("scan");
    const bool inspect = parser.isSet("inspect-gatt");
    const bool raw = parser.isSet("debug-raw");
    const bool mouse = parser.isSet("mouse");
    const auto error = [](const QString& message) {
        QTextStream(stderr) << "Error: " << message << Qt::endl;
        return 2;
    };
    if (!parser.positionalArguments().isEmpty()) return error("Unexpected positional arguments. Use --help.");
    if (scan && (inspect || raw || mouse || parser.isSet("device")))
        return error("--scan cannot be combined with --inspect-gatt, --debug-raw, --mouse, or --device.");
    if (inspect && raw) return error("--inspect-gatt does not subscribe; use --debug-raw in streaming mode.");
    if (inspect && mouse) return error("--mouse requires streaming and cannot be combined with --inspect-gatt.");
#ifndef Q_OS_WIN
    if (mouse) return error("--mouse is currently supported only on Windows.");
#endif
    QString identifier;
    if (parser.isSet("device")) {
        identifier = pokepointer::canonicalIdentifier(parser.value("device"));
        if (identifier.isEmpty()) return error("--device requires a valid Bluetooth address or UUID from --scan.");
    }
    pokepointer::DeviceStore store;
    if (parser.isSet("forget-device")) {
        if (!store.forget()) return error("Could not clear the stored device selection.");
        QTextStream(stdout) << "Saved device selection cleared." << Qt::endl;
        if (!scan && !inspect && !raw && !mouse && identifier.isEmpty()) return 0;
    }
    const auto cached = pokepointer::canonicalIdentifier(store.load());
    pokepointer::Application coordinator({scan, inspect, identifier, cached}, raw, mouse, store);
    QTimer::singleShot(0, &coordinator, &pokepointer::Application::start);
    return app.exec();
}
