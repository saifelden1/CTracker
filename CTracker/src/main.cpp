#include <QApplication>
#include <QMessageBox>

#include "DatabaseManager.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CTracker"));
    QApplication::setOrganizationName(QStringLiteral("CTracker"));

    auto* db = DatabaseManager::instance();
    if (!db->initialize()) {
        QMessageBox::critical(nullptr,
            QObject::tr("CTracker"),
            QObject::tr("Failed to initialize the database. The application will exit."));
        return 1;
    }

    MainWindow window;
    window.show();

    const int rc = app.exec();
    db->close();
    return rc;
}
