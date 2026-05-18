#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QComboBox;
class QTextEdit;
class QDateEdit;
class QPushButton;
class QLabel;

// Task 7.10: EntityCreateDialog — modal dialog for creating a Course or Project.
// Two modes:
//   - CourseOrProject: shows a type selector (Course / Project), used from CoursesView
//   - ProjectOnly: hides the type selector, used from ProjectsView
// Validates Name not empty/whitespace → enables OK button.
// On accept, calls addCourse/addProject and (for projects) upsertProjectMeta
// with description/priority/deadline.
class EntityCreateDialog : public QDialog {
    Q_OBJECT
public:
    enum class Mode {
        CourseOrProject,   // from CoursesView — user picks type
        ProjectOnly        // from ProjectsView — type is forced to Project
    };

    explicit EntityCreateDialog(Mode mode, QWidget* parent = nullptr);

    // Results (valid after dialog accepted)
    QString entityType() const;     // "course" or "project"
    QString entityName() const;
    int     createdEntityId() const; // ID of the newly created entity (-1 if rejected)
    QString description() const;    // project-only
    QString priority() const;       // project-only: "high"/"medium"/"low"
    QDate   deadline() const;       // project-only
    int     selectedCategoryId() const; // -1 = no category selected

private slots:
    void onNameChanged(const QString& text);
    void onTypeChanged(int index);
    void onAccept();

private:
    void setupUi();
    void updateProjectFieldsVisibility();

    Mode m_mode;

    // Common fields
    QLineEdit*   m_nameEdit      = nullptr;
    QComboBox*   m_typeCombo     = nullptr;   // only visible in CourseOrProject mode
    QPushButton* m_okBtn         = nullptr;
    QPushButton* m_cancelBtn     = nullptr;

    // Category field
    QLabel*      m_categoryLabel = nullptr;
    QComboBox*   m_categoryCombo = nullptr;

    // Project-specific fields
    QLabel*      m_descLabel     = nullptr;
    QTextEdit*   m_descEdit      = nullptr;
    QLabel*      m_priorityLabel = nullptr;
    QComboBox*   m_priorityCombo = nullptr;
    QLabel*      m_deadlineLabel = nullptr;
    QDateEdit*   m_deadlineEdit  = nullptr;

    QString m_entityType = "course";
    int     m_createdId  = -1;
};