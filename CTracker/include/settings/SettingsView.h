#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class DataImporter;
class DataExporter;

class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView(QWidget* parent = nullptr);

private slots:
    void onImportClicked();
    void onExportClicked();

private:
    void setupUi();
    QString resolveDatabasePath() const;

    QPushButton* m_importBtn  = nullptr;
    QPushButton* m_exportBtn  = nullptr;
    QLabel*      m_dbPathLabel = nullptr;

    DataImporter* m_importer = nullptr;
    DataExporter* m_exporter = nullptr;
};
