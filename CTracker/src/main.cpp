#include <QApplication>
#include <QMessageBox>

#include "core/DatabaseManager.h"
#include "shared/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CTracker"));
    QApplication::setOrganizationName(QStringLiteral("CTracker"));

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
    db->close();
    return rc;
}
