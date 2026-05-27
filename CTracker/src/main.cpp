#include <QApplication>
#include <QStyleFactory>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>
#include <QtGlobal>

#include "core/DatabaseManager.h"
#include "shared/MainWindow.h"

static void ctrackerMessageHandler(QtMsgType type,
                                   const QMessageLogContext& context,
                                   const QString& message)
{
    if (type == QtDebugMsg && !qEnvironmentVariableIsSet("CTRACKER_DEBUG_LOG")) {
        return;
    }

    static QMutex mutex;
    QMutexLocker locker(&mutex);

    const QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logDir);

    const QString logPath = logDir + QStringLiteral("/ctracker-debug.log");
    const QString oldLogPath = logDir + QStringLiteral("/ctracker-debug.old.log");
    constexpr qint64 maxLogBytes = 5 * 1024 * 1024;

    QFile existing(logPath);
    if (existing.exists() && existing.size() >= maxLogBytes) {
        QFile::remove(oldLogPath);
        existing.rename(oldLogPath);
    }

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    const char* level = "DEBUG";
    if (type == QtInfoMsg) level = "INFO";
    else if (type == QtWarningMsg) level = "WARN";
    else if (type == QtCriticalMsg) level = "CRITICAL";
    else if (type == QtFatalMsg) level = "FATAL";

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
        << " [" << level << "] " << message;

    if (context.file) {
        out << " (" << context.file << ":" << context.line << ")";
    }
    out << '\n';
    out.flush();
}

int main(int argc, char* argv[]) {
    qInstallMessageHandler(ctrackerMessageHandler);

    QApplication app(argc, argv);
    // Force Fusion style. The default qmodernwindowsstyle plugin on Win11
    // re-polishes top-level popups (QComboBox dropdown, QMenu) after QSS is
    // applied, overriding the background to mimic the OS's translucent menus
    // — which is why our dropdowns "started right then went transparent".
    // Fusion respects QSS fully and paints opaque.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    QApplication::setApplicationName(QStringLiteral("CTracker"));
    QApplication::setOrganizationName(QStringLiteral("CTracker"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app-icon.png")));

    qInfo() << "[APP] starting";

    auto* db = DatabaseManager::instance();
    if (!db->initialize()) {
        QMessageBox::critical(nullptr,
            QObject::tr("Database Error"),
            QObject::tr("Failed to initialize the database.\n\nThe application will exit."));
        return 1;
    }

    MainWindow* window = new MainWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    window->raise();
    window->activateWindow();

    const int rc = app.exec();
    qInfo() << "[APP] exiting" << rc;
    db->close();
    return rc;
}
