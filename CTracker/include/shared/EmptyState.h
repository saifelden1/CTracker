#pragma once

#include <QWidget>
#include <QString>

class QLabel;
class QPushButton;

// EmptyState: a centered placeholder shown when a view has no content.
// Displays an icon circle, title, description, and optional action button.
// Used by CoursesView, ProjectsView, and any future list view.
class EmptyState : public QWidget {
    Q_OBJECT
public:
    explicit EmptyState(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setActionLabel(const QString& label);
    void setShowAction(bool show);

signals:
    void actionRequested();

private:
    void setupUi();

    QLabel*      m_iconLabel        = nullptr;
    QLabel*      m_titleLabel       = nullptr;
    QLabel*      m_descriptionLabel = nullptr;
    QPushButton* m_actionBtn        = nullptr;
};