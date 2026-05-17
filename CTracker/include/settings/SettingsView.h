#pragma once

#include <QWidget>
#include <QDate>

class QLineEdit;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QFrame;
class DataImporter;
class DataExporter;
class CategoryModel;
class QListView;

// Task 7.11: SettingsView — v2 with 5 cards.
// Profile: Name, Email, Study Goals — bound via getProfile/setProfile
// Preferences: pomodoro durations, notifications, sound, autoPauseDays — via getPreferences/setPreferences
// Categories: list with edit/add + color picker dialog; uses CategoryModel
// Data Management: Export, Import, Clear All Data (destructive red, confirmation dialog)
// About: version, build, license
class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView(QWidget* parent = nullptr);

private slots:
    void onSaveProfile();
    void onSavePreferences();
    void onAddCategory();
    void onEditCategory();
    void onDeleteCategory();
    void onImportClicked();
    void onExportClicked();
    void onClearAllData();
    void onDataChanged();

private:
    void setupUi();
    void setupProfileCard();
    void setupPreferencesCard();
    void setupCategoriesCard();
    void setupDataManagementCard();
    void setupAboutCard();
    void refreshProfile();
    void refreshPreferences();
    void refreshCategories();
    QString resolveDatabasePath() const;

    // ── Profile card ──
    QFrame*      m_profileCard      = nullptr;
    QLineEdit*   m_nameEdit         = nullptr;
    QLineEdit*   m_emailEdit        = nullptr;
    QTextEdit*   m_goalsEdit        = nullptr;
    QPushButton* m_saveProfileBtn   = nullptr;

    // ── Preferences card ──
    QFrame*      m_preferencesCard  = nullptr;
    QComboBox*   m_workDurationCombo = nullptr;
    QComboBox*   m_breakDurationCombo = nullptr;
    QCheckBox*   m_notificationsCheck = nullptr;
    QCheckBox*   m_soundCheck        = nullptr;
    QComboBox*   m_autoPauseCombo    = nullptr;
    QPushButton* m_savePreferencesBtn = nullptr;

    // ── Categories card ──
    QFrame*      m_categoriesCard   = nullptr;
    CategoryModel* m_categoryModel  = nullptr;
    QListView*   m_categoryList     = nullptr;
    QPushButton* m_addCategoryBtn   = nullptr;
    QPushButton* m_editCategoryBtn  = nullptr;
    QPushButton* m_deleteCategoryBtn = nullptr;

    // ── Data Management card ──
    QFrame*      m_dataCard         = nullptr;
    QPushButton* m_importBtn        = nullptr;
    QPushButton* m_exportBtn        = nullptr;
    QPushButton* m_clearAllBtn      = nullptr;
    QLabel*      m_dbPathLabel      = nullptr;

    // ── About card ──
    QFrame*      m_aboutCard        = nullptr;

    // ── Helpers ──
    DataImporter* m_importer = nullptr;
    DataExporter* m_exporter = nullptr;
};
