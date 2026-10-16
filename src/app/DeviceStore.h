#pragma once

#include <QString>

namespace pokepointer {
class DeviceStore {
public:
    explicit DeviceStore(QString fileName = {});
    [[nodiscard]] QString load() const;
    [[nodiscard]] bool save(const QString& identifier) const;
    [[nodiscard]] bool forget() const;
    [[nodiscard]] const QString& fileName() const { return fileName_; }
private:
    QString fileName_;
};
} // namespace pokepointer
