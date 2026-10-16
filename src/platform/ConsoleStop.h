#pragma once

#include <QObject>
#include <QTimer>
#include <csignal>

namespace pokepointer {
// One instance per process. OS handlers only set a flag; Qt performs shutdown.
class ConsoleStop final : public QObject {
    Q_OBJECT
public:
    explicit ConsoleStop(QObject* parent = nullptr);
    ~ConsoleStop() override;
signals:
    void requested();
private:
    QTimer poll_;
#ifdef Q_OS_WIN
    bool installed_{};
#else
    using Handler = void (*)(int);
    Handler previousInterrupt_{};
    Handler previousTerminate_{};
#endif
};
} // namespace pokepointer
