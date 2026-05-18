#include "settings/SettingsView.h"

#include "core/DataImporter.h"
#include "core/DataExporter.h"
#include "core/DatabaseManager.h"
#include "core/DataStructures.h"
#include "shared/CategoryModel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QListView>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QDir>
#include <QColorDialog>
#include <QFont>

// Task 7.11: SettingsView — v2 with 5 cards.

SettingsView::SettingsView(QWidget* parent)
    : QWidget(parent),
      m_importer(new DataImporter(this)),
      m_exporter(new DataExporter(this)),
      m_categoryModel(new CategoryModel(this)) {
    setObjectName("settingsView");
    setupUi();

    // Wire importer/exporter signals
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

    // Live refresh on DB changes
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &SettingsView::onDataChanged);

    refreshProfile();
    refreshPreferences();
    refreshCategories();
}

void SettingsView::setupUi() {
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* container = new QWidget(scrollArea);
    auto* outer = new QVBoxLayout(container);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(24);

    // ── Header ──
    auto* headerLabel = new QLabel(tr("Settings"), container);
    headerLabel->setObjectName("settingsTitle");
    QFont titleFont = headerLabel->font();
    titleFont.setWeight(QFont::Medium);
    titleFont.setPointSize(18);
    headerLabel->setFont(titleFont);
    outer->addWidget(headerLabel);

    auto* subtitleLabel = new QLabel(tr("Manage your preferences and app configuration"), container);
    subtitleLabel->setObjectName("settingsSubtitle");
    outer->addWidget(subtitleLabel);

    // ── 5 Cards ──
    setupProfileCard();
    outer->addWidget(m_profileCard);

    setupPreferencesCard();
    outer->addWidget(m_preferencesCard);

    setupCategoriesCard();
    outer->addWidget(m_categoriesCard);

    setupDataManagementCard();
    outer->addWidget(m_dataCard);

    setupAboutCard();
    outer->addWidget(m_aboutCard);

    outer->addStretch(1);

    scrollArea->setWidget(container);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
}

// ── Helper: make a card frame ──
static QFrame* makeCard(const QString& objectName, QWidget* parent) {
    auto* card = new QFrame(parent);
    card->setObjectName(objectName);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    return card;
}

static QLabel* makeCardTitle(const QString& text, const QString& objectName, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName(objectName);
    QFont font = label->font();
    font.setWeight(QFont::Medium);
    font.setPointSize(14);
    label->setFont(font);
    return label;
}

void SettingsView::setupProfileCard() {
    m_profileCard = makeCard("settingsProfileCard", this);
    auto* layout = static_cast<QVBoxLayout*>(m_profileCard->layout());

    layout->addWidget(makeCardTitle(tr("Profile"), "settingsCardTitle", m_profileCard));

    // Name
    auto* nameRow = new QHBoxLayout();
    nameRow->setSpacing(8);
    auto* nameLabel = new QLabel(tr("Name:"), m_profileCard);
    m_nameEdit = new QLineEdit(m_profileCard);
    m_nameEdit->setObjectName("settingsNameEdit");
    m_nameEdit->setPlaceholderText(tr("Your name"));
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(m_nameEdit, 1);
    layout->addLayout(nameRow);

    // Email
    auto* emailRow = new QHBoxLayout();
    emailRow->setSpacing(8);
    auto* emailLabel = new QLabel(tr("Email:"), m_profileCard);
    m_emailEdit = new QLineEdit(m_profileCard);
    m_emailEdit->setObjectName("settingsEmailEdit");
    m_emailEdit->setPlaceholderText(tr("your.email@example.com"));
    emailRow->addWidget(emailLabel);
    emailRow->addWidget(m_emailEdit, 1);
    layout->addLayout(emailRow);

    // Study Goals
    auto* goalsLabel = new QLabel(tr("Study Goals:"), m_profileCard);
    layout->addWidget(goalsLabel);
    m_goalsEdit = new QTextEdit(m_profileCard);
    m_goalsEdit->setObjectName("settingsGoalsEdit");
    m_goalsEdit->setMaximumHeight(100);
    m_goalsEdit->setPlaceholderText(tr("What are your learning goals?"));
    layout->addWidget(m_goalsEdit);

    m_saveProfileBtn = new QPushButton(tr("Save Profile"), m_profileCard);
    m_saveProfileBtn->setObjectName("settingsSaveBtn");
    m_saveProfileBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveProfileBtn, &QPushButton::clicked, this, &SettingsView::onSaveProfile);
    layout->addWidget(m_saveProfileBtn);
}

void SettingsView::setupPreferencesCard() {
    m_preferencesCard = makeCard("settingsPreferencesCard", this);
    auto* layout = static_cast<QVBoxLayout*>(m_preferencesCard->layout());

    layout->addWidget(makeCardTitle(tr("Preferences"), "settingsCardTitle", m_preferencesCard));

    // Pomodoro Timer section
    auto* pomodoroLabel = new QLabel(tr("Pomodoro Timer"), m_preferencesCard);
    pomodoroLabel->setObjectName("settingsSectionLabel");
    QFont sectionFont = pomodoroLabel->font();
    sectionFont.setWeight(QFont::Medium);
    pomodoroLabel->setFont(sectionFont);
    layout->addWidget(pomodoroLabel);

    auto* durationRow = new QHBoxLayout();
    durationRow->setSpacing(16);

    // Work duration
    auto* workBox = new QVBoxLayout();
    workBox->setSpacing(4);
    auto* workLabel = new QLabel(tr("Default Work Duration"), m_preferencesCard);
    workLabel->setObjectName("settingsFieldLabel");
    m_workDurationCombo = new QComboBox(m_preferencesCard);
    m_workDurationCombo->setObjectName("settingsWorkDuration");
    for (int min : {15, 20, 25, 30, 45, 50}) {
        m_workDurationCombo->addItem(tr("%1 minutes").arg(min), min);
    }
    workBox->addWidget(workLabel);
    workBox->addWidget(m_workDurationCombo);
    durationRow->addLayout(workBox);

    // Break duration
    auto* breakBox = new QVBoxLayout();
    breakBox->setSpacing(4);
    auto* breakLabel = new QLabel(tr("Default Break Duration"), m_preferencesCard);
    breakLabel->setObjectName("settingsFieldLabel");
    m_breakDurationCombo = new QComboBox(m_preferencesCard);
    m_breakDurationCombo->setObjectName("settingsBreakDuration");
    for (int min : {5, 10, 15}) {
        m_breakDurationCombo->addItem(tr("%1 minutes").arg(min), min);
    }
    breakBox->addWidget(breakLabel);
    breakBox->addWidget(m_breakDurationCombo);
    durationRow->addLayout(breakBox);

    layout->addLayout(durationRow);

    // Notifications & Sounds section
    auto* notifLabel = new QLabel(tr("Notifications & Sounds"), m_preferencesCard);
    notifLabel->setObjectName("settingsSectionLabel");
    notifLabel->setFont(sectionFont);
    layout->addWidget(notifLabel);

    m_notificationsCheck = new QCheckBox(tr("Enable Notifications"), m_preferencesCard);
    m_notificationsCheck->setObjectName("settingsNotificationsCheck");
    layout->addWidget(m_notificationsCheck);

    m_soundCheck = new QCheckBox(tr("Sound Effects"), m_preferencesCard);
    m_soundCheck->setObjectName("settingsSoundCheck");
    layout->addWidget(m_soundCheck);

    // Auto-pause section
    auto* autoPauseLabel = new QLabel(tr("Course Management"), m_preferencesCard);
    autoPauseLabel->setObjectName("settingsSectionLabel");
    autoPauseLabel->setFont(sectionFont);
    layout->addWidget(autoPauseLabel);

    auto* autoPauseRow = new QHBoxLayout();
    autoPauseRow->setSpacing(8);
    auto* autoPauseDesc = new QLabel(tr("Auto-pause inactive courses after"), m_preferencesCard);
    autoPauseDesc->setObjectName("settingsFieldLabel");
    m_autoPauseCombo = new QComboBox(m_preferencesCard);
    m_autoPauseCombo->setObjectName("settingsAutoPauseCombo");
    m_autoPauseCombo->addItem(tr("7 days"), 7);
    m_autoPauseCombo->addItem(tr("14 days"), 14);
    m_autoPauseCombo->addItem(tr("30 days"), 30);
    m_autoPauseCombo->addItem(tr("Never"), 0);
    autoPauseRow->addWidget(autoPauseDesc);
    autoPauseRow->addWidget(m_autoPauseCombo, 1);
    layout->addLayout(autoPauseRow);

    m_savePreferencesBtn = new QPushButton(tr("Save Preferences"), m_preferencesCard);
    m_savePreferencesBtn->setObjectName("settingsSaveBtn");
    m_savePreferencesBtn->setCursor(Qt::PointingHandCursor);
    connect(m_savePreferencesBtn, &QPushButton::clicked, this, &SettingsView::onSavePreferences);
    layout->addWidget(m_savePreferencesBtn);
}

void SettingsView::setupCategoriesCard() {
    m_categoriesCard = makeCard("settingsCategoriesCard", this);
    auto* layout = static_cast<QVBoxLayout*>(m_categoriesCard->layout());

    layout->addWidget(makeCardTitle(tr("Course Categories"), "settingsCardTitle", m_categoriesCard));

    // Category list view
    m_categoryList = new QListView(m_categoriesCard);
    m_categoryList->setObjectName("settingsCategoryList");
    m_categoryList->setModel(m_categoryModel);
    m_categoryList->setEditTriggers(QListView::NoEditTriggers);
    m_categoryList->setMinimumHeight(120);
    m_categoryList->setMaximumHeight(200);
    layout->addWidget(m_categoryList);

    // Buttons row
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);

    m_addCategoryBtn = new QPushButton(tr("Add Category"), m_categoriesCard);
    m_addCategoryBtn->setObjectName("settingsAddCategoryBtn");
    m_addCategoryBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addCategoryBtn, &QPushButton::clicked, this, &SettingsView::onAddCategory);
    btnRow->addWidget(m_addCategoryBtn);

    m_editCategoryBtn = new QPushButton(tr("Edit"), m_categoriesCard);
    m_editCategoryBtn->setObjectName("settingsEditCategoryBtn");
    m_editCategoryBtn->setCursor(Qt::PointingHandCursor);
    connect(m_editCategoryBtn, &QPushButton::clicked, this, &SettingsView::onEditCategory);
    btnRow->addWidget(m_editCategoryBtn);

    m_deleteCategoryBtn = new QPushButton(tr("Delete"), m_categoriesCard);
    m_deleteCategoryBtn->setObjectName("settingsDeleteCategoryBtn");
    m_deleteCategoryBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteCategoryBtn, &QPushButton::clicked, this, &SettingsView::onDeleteCategory);
    btnRow->addWidget(m_deleteCategoryBtn);

    btnRow->addStretch(1);
    layout->addLayout(btnRow);
}

void SettingsView::setupDataManagementCard() {
    m_dataCard = makeCard("settingsDataCard", this);
    auto* layout = static_cast<QVBoxLayout*>(m_dataCard->layout());

    layout->addWidget(makeCardTitle(tr("Data Management"), "settingsCardTitle", m_dataCard));

    // Export / Import buttons
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);

    m_exportBtn = new QPushButton(tr("Export Data"), m_dataCard);
    m_exportBtn->setObjectName("settingsExportBtn");
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &SettingsView::onExportClicked);
    btnRow->addWidget(m_exportBtn);

    m_importBtn = new QPushButton(tr("Import Data"), m_dataCard);
    m_importBtn->setObjectName("settingsImportBtn");
    m_importBtn->setCursor(Qt::PointingHandCursor);
    connect(m_importBtn, &QPushButton::clicked, this, &SettingsView::onImportClicked);
    btnRow->addWidget(m_importBtn);

    btnRow->addStretch(1);
    layout->addLayout(btnRow);

    // Database location
    auto* dbLabel = new QLabel(tr("Database Location:"), m_dataCard);
    dbLabel->setObjectName("settingsFieldLabel");
    layout->addWidget(dbLabel);
    m_dbPathLabel = new QLabel(resolveDatabasePath(), m_dataCard);
    m_dbPathLabel->setObjectName("settingsDbPath");
    m_dbPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_dbPathLabel->setWordWrap(true);
    layout->addWidget(m_dbPathLabel);

    // Danger zone separator
    auto* separator = new QFrame(m_dataCard);
    separator->setObjectName("settingsSeparator");
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);
    layout->addWidget(separator);

    auto* dangerLabel = new QLabel(tr("Danger Zone: This action cannot be undone"), m_dataCard);
    dangerLabel->setObjectName("settingsDangerLabel");
    layout->addWidget(dangerLabel);

    m_clearAllBtn = new QPushButton(tr("Clear All Data"), m_dataCard);
    m_clearAllBtn->setObjectName("settingsClearAllBtn");
    m_clearAllBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearAllBtn, &QPushButton::clicked, this, &SettingsView::onClearAllData);
    layout->addWidget(m_clearAllBtn);
}

void SettingsView::setupAboutCard() {
    m_aboutCard = makeCard("settingsAboutCard", this);
    auto* layout = static_cast<QVBoxLayout*>(m_aboutCard->layout());

    layout->addWidget(makeCardTitle(tr("About"), "settingsCardTitle", m_aboutCard));

    // Version row
    auto* versionRow = new QHBoxLayout();
    auto* versionLabel = new QLabel(tr("Version"), m_aboutCard);
    versionLabel->setObjectName("settingsAboutField");
    auto* versionValue = new QLabel(QStringLiteral("1.0.0"), m_aboutCard);
    versionValue->setObjectName("settingsAboutValue");
    versionRow->addWidget(versionLabel);
    versionRow->addStretch(1);
    versionRow->addWidget(versionValue);
    layout->addLayout(versionRow);

    // Built with row
    auto* builtRow = new QHBoxLayout();
    auto* builtLabel = new QLabel(tr("Built with"), m_aboutCard);
    builtLabel->setObjectName("settingsAboutField");
    auto* builtValue = new QLabel(tr("C++17 / Qt 6 / SQLite"), m_aboutCard);
    builtValue->setObjectName("settingsAboutValue");
    builtRow->addWidget(builtLabel);
    builtRow->addStretch(1);
    builtRow->addWidget(builtValue);
    layout->addLayout(builtRow);

    // License row
    auto* licenseRow = new QHBoxLayout();
    auto* licenseLabel = new QLabel(tr("License"), m_aboutCard);
    licenseLabel->setObjectName("settingsAboutField");
    auto* licenseValue = new QLabel(tr("MIT"), m_aboutCard);
    licenseValue->setObjectName("settingsAboutValue");
    licenseRow->addWidget(licenseLabel);
    licenseRow->addStretch(1);
    licenseRow->addWidget(licenseValue);
    layout->addLayout(licenseRow);
}

// ── Data refresh ──

void SettingsView::refreshProfile() {
    auto* db = DatabaseManager::instance();
    const ProfileData profile = db->getProfile();
    m_nameEdit->setText(profile.name);
    m_emailEdit->setText(profile.email);
    m_goalsEdit->setPlainText(profile.goals);
}

void SettingsView::refreshPreferences() {
    auto* db = DatabaseManager::instance();
    const PreferencesData prefs = db->getPreferences();

    // Set work duration combo
    for (int i = 0; i < m_workDurationCombo->count(); ++i) {
        if (m_workDurationCombo->itemData(i).toInt() == prefs.workMinutes) {
            m_workDurationCombo->setCurrentIndex(i);
            break;
        }
    }

    // Set break duration combo
    for (int i = 0; i < m_breakDurationCombo->count(); ++i) {
        if (m_breakDurationCombo->itemData(i).toInt() == prefs.breakMinutes) {
            m_breakDurationCombo->setCurrentIndex(i);
            break;
        }
    }

    m_notificationsCheck->setChecked(prefs.notifications);
    m_soundCheck->setChecked(prefs.sound);

    // Set auto-pause combo
    for (int i = 0; i < m_autoPauseCombo->count(); ++i) {
        if (m_autoPauseCombo->itemData(i).toInt() == prefs.autoPauseDays) {
            m_autoPauseCombo->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsView::refreshCategories() {
    m_categoryModel->refresh();
}

// ── Slots ──

void SettingsView::onSaveProfile() {
    auto* db = DatabaseManager::instance();
    ProfileData profile;
    profile.name  = m_nameEdit->text().trimmed();
    profile.email = m_emailEdit->text().trimmed();
    profile.goals = m_goalsEdit->toPlainText().trimmed();
    db->setProfile(profile);
}

void SettingsView::onSavePreferences() {
    auto* db = DatabaseManager::instance();
    PreferencesData prefs;
    prefs.workMinutes   = m_workDurationCombo->currentData().toInt();
    prefs.breakMinutes  = m_breakDurationCombo->currentData().toInt();
    prefs.notifications = m_notificationsCheck->isChecked();
    prefs.sound         = m_soundCheck->isChecked();
    prefs.autoPauseDays = m_autoPauseCombo->currentData().toInt();
    db->setPreferences(prefs);
}

void SettingsView::onAddCategory() {
    // Ask for name
    QString name;
    QColor color = QColor("#10b981");

    // Simple dialog: name + color
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Add Category"));
    dialog->setMinimumWidth(300);
    auto* dlgLayout = new QVBoxLayout(dialog);
    dlgLayout->setSpacing(12);

    auto* nameEdit = new QLineEdit(dialog);
    nameEdit->setPlaceholderText(tr("Category name"));
    dlgLayout->addWidget(nameEdit);

    auto* colorBtn = new QPushButton(tr("Choose Color..."), dialog);
    dlgLayout->addWidget(colorBtn);

    auto* colorPreview = new QLabel(dialog);
    colorPreview->setFixedSize(24, 24);
    colorPreview->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(color.name()));
    dlgLayout->addWidget(colorPreview);

    connect(colorBtn, &QPushButton::clicked, this, [&, colorPreview]() {
        const QColor chosen = QColorDialog::getColor(color, this, tr("Category Color"));
        if (chosen.isValid()) {
            color = chosen;
            colorPreview->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(color.name()));
        }
    });

    auto* btnRow = new QHBoxLayout();
    auto* cancelBtn = new QPushButton(tr("Cancel"), dialog);
    auto* okBtn = new QPushButton(tr("Add"), dialog);
    okBtn->setEnabled(false);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(nameEdit, &QLineEdit::textChanged, this, [okBtn](const QString& text) {
        okBtn->setEnabled(!text.trimmed().isEmpty());
    });
    btnRow->addStretch(1);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    dlgLayout->addLayout(btnRow);

    if (dialog->exec() == QDialog::Accepted) {
        name = nameEdit->text().trimmed();
        if (!name.isEmpty()) {
            DatabaseManager::instance()->addCategory(name, color);
        }
    }
    dialog->deleteLater();
}

void SettingsView::onEditCategory() {
    const QModelIndex idx = m_categoryList->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Edit Category"), tr("Select a category first."));
        return;
    }

    const int categoryId = m_categoryModel->data(idx, CategoryModel::IdRole).toInt();
    const QString oldName = m_categoryModel->data(idx, CategoryModel::NameRole).toString();
    const QColor oldColor = m_categoryModel->data(idx, CategoryModel::ColorRole).value<QColor>();

    auto* dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Edit Category"));
    dialog->setMinimumWidth(300);
    auto* dlgLayout = new QVBoxLayout(dialog);
    dlgLayout->setSpacing(12);

    auto* nameEdit = new QLineEdit(oldName, dialog);
    dlgLayout->addWidget(nameEdit);

    QColor color = oldColor;
    auto* colorBtn = new QPushButton(tr("Choose Color..."), dialog);
    dlgLayout->addWidget(colorBtn);

    auto* colorPreview = new QLabel(dialog);
    colorPreview->setFixedSize(24, 24);
    colorPreview->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(color.name()));
    dlgLayout->addWidget(colorPreview);

    connect(colorBtn, &QPushButton::clicked, this, [&, colorPreview]() {
        const QColor chosen = QColorDialog::getColor(color, this, tr("Category Color"));
        if (chosen.isValid()) {
            color = chosen;
            colorPreview->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(color.name()));
        }
    });

    auto* btnRow = new QHBoxLayout();
    auto* cancelBtn = new QPushButton(tr("Cancel"), dialog);
    auto* okBtn = new QPushButton(tr("Save"), dialog);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    btnRow->addStretch(1);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    dlgLayout->addLayout(btnRow);

    if (dialog->exec() == QDialog::Accepted) {
        const QString newName = nameEdit->text().trimmed();
        if (!newName.isEmpty()) {
            auto* db = DatabaseManager::instance();
            db->renameCategory(categoryId, newName);
            db->setCategoryColor(categoryId, color);
        }
    }
    dialog->deleteLater();
}

void SettingsView::onDeleteCategory() {
    const QModelIndex idx = m_categoryList->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Delete Category"), tr("Select a category first."));
        return;
    }

    const int categoryId = m_categoryModel->data(idx, CategoryModel::IdRole).toInt();
    const QString name = m_categoryModel->data(idx, CategoryModel::NameRole).toString();

    const int result = QMessageBox::question(this,
        tr("Delete Category"),
        tr("Are you sure you want to delete \"%1\"?\nCourses/projects using this category will be unlinked.")
            .arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        DatabaseManager::instance()->removeCategory(categoryId);
    }
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

void SettingsView::onClearAllData() {
    const int result = QMessageBox::warning(this,
        tr("Clear All Data"),
        tr("This will permanently delete ALL your courses, projects, todos, pomodoro sessions, and settings.\n\nThis action CANNOT be undone.\n\nAre you absolutely sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    // Second confirmation
    const int result2 = QMessageBox::critical(this,
        tr("Final Confirmation"),
        tr("Type \"Yes\" to confirm.\nThis is your last chance to cancel."),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if (result2 != QMessageBox::Yes) {
        return;
    }

    auto* db = DatabaseManager::instance();
    const bool success = db->clearAllData();
    if (success) {
        refreshProfile();
        refreshPreferences();
        refreshCategories();
        m_dbPathLabel->setText(resolveDatabasePath());
        QMessageBox::information(this, tr("Cleared"),
            tr("All data has been successfully cleared."));
    } else {
        QMessageBox::warning(this, tr("Error"),
            tr("Failed to clear all data. Please try again."));
    }
}

void SettingsView::onDataChanged() {
    refreshProfile();
    refreshPreferences();
    refreshCategories();
    m_dbPathLabel->setText(resolveDatabasePath());
}

QString SettingsView::resolveDatabasePath() const {
    const QSqlDatabase db = QSqlDatabase::database();
    if (db.isValid() && !db.databaseName().isEmpty()) {
        return QDir::toNativeSeparators(db.databaseName());
    }
    return tr("(not yet initialized)");
}
