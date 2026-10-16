#include "app/DeviceStore.h"
#include <QSettings>
#include <QStandardPaths>
#include <utility>

namespace pokepointer {
DeviceStore::DeviceStore(QString fileName) : fileName_(std::move(fileName))
{
    if (fileName_.isEmpty())
        fileName_ = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                  + "/device.ini";
}

QString DeviceStore::load() const
{
    QSettings settings(fileName_, QSettings::IniFormat);
    return settings.value("device/identifier").toString();
}

bool DeviceStore::save(const QString& identifier) const
{
    QSettings settings(fileName_, QSettings::IniFormat);
    settings.setValue("device/identifier", identifier);
    settings.sync();
    return settings.status() == QSettings::NoError;
}

bool DeviceStore::forget() const
{
    QSettings settings(fileName_, QSettings::IniFormat);
    settings.remove("device");
    settings.sync();
    return settings.status() == QSettings::NoError;
}
} // namespace pokepointer
