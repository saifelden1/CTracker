#include "SettingsView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QDir>

#include "DataImporter.h"
#include "DataExporter.h"
#include "DatabaseManager.h"

SettingsView::SettingsView(QWidget* parent)
    : QWidget(parent),
      m_importer(new DataImporter(this)),
      m_exporter(new DataExporter(this)) {
    setupUi();

    connect(m_importer, &DataImporter::importFailed, this,
        [this](const QString& reason) {
            QMessageBox::warning(this, tr("Import failed"), reason);
        });
    connect(m_importer, &DataImporter::importCompleted, this,
        [this](int entityId) {
            Q_UNUSED(entityId);
            QMessageBox::information(this, tr("Import complete"),
                tr("Data imported successfully."));
        });
    connect(m_exporter, &DataExporter::exportFailed, this,
        [this](const QString& reason) {
            QMessageBox::warning(this, tr("Export failed"), reason);
        });
    connect(m_exporter, &DataExporter::exportCompleted, this,
        [this](const QString& path) {
            QMessageBox::information(this, tr("Export complete"),
                tr("Saved to:\n%1").arg(path));
        });
}

void SettingsView::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 16, 16, 16);
    outer->setSpacing(12);

    auto* title = new QLabel(tr("Settings"), this);
    title->setObjectName("settingsTitle");
    outer->addWidget(title);

    // ---- Data Management ----
    auto* dataBox = new QGroupBox(tr("Data Management"), this);
    auto* dataLayout = new QHBoxLayout(dataBox);
    dataLayout->setSpacing(8);

    m_importBtn = new QPushButton(tr("Import Data..."), dataBox);
    m_exportBtn = new QPushButton(tr("Export Data..."), dataBox);

    dataLayout->addWidget(m_importBtn);
    dataLayout->addWidget(m_exportBtn);
    dataLayout->addStretch(1);

    outer->addWidget(dataBox);

    // ---- Database location ----
    auto* dbBox = new QGroupBox(tr("Database Location"), this);
    auto* dbLayout = new QVBoxLayout(dbBox);
    m_dbPathLabel = new QLabel(resolveDatabasePath(), dbBox);
    m_dbPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_dbPathLabel->setWordWrap(true);
    dbLayout->addWidget(m_dbPathLabel);
    outer->addWidget(dbBox);

    outer->addStretch(1);

    connect(m_importBtn, &QPushButton::clicked, this, &SettingsView::onImportClicked);
    connect(m_exportBtn, &QPushButton::clicked, this, &SettingsView::onExportClicked);
}

QString SettingsView::resolveDatabasePath() const {
    const QSqlDatabase db = QSqlDatabase::database();
    if (db.isValid() && !db.databaseName().isEmpty()) {
        return QDir::toNativeSeparators(db.databaseName());
    }
    return tr("(not yet initialized)");
}

void SettingsView::onImportClicked() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Import Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("JSON files (*.json);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    if (!m_importer->importFromFile(path)) {
        // The importFailed signal already surfaces the message; this is a
        // safety net for direct callers.
        const QString last = m_importer->lastError();
        if (!last.isEmpty()) {
            QMessageBox::warning(this, tr("Import failed"), last);
        }
    }
}

void SettingsView::onExportClicked() {
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Export Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + QStringLiteral("/ctracker-export.json"),
        tr("JSON files (*.json);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    if (!m_exporter->exportToFile(path)) {
        const QString last = m_exporter->lastError();
        if (!last.isEmpty()) {
            QMessageBox::warning(this, tr("Export failed"), last);
        }
    }
}
