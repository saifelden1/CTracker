#pragma once

#include <QFrame>
#include <QString>
#include <QDate>

class QLabel;
class QProgressBar;

// ProjectCard: a card widget for displaying a project with metadata.
// Layout: name (semibold) + description (2-line truncate) + priority badge +
// deadline badge + horizontal progress bar + "X/Y tasks" + team-icon + member count.
// Deadline badge computes days-left with color coding:
//   ≤ 3 d → red, ≤ 7 d → amber, > 7 d → gray, no deadline → hidden.
class ProjectCard : public QFrame {
    Q_OBJECT
public:
    explicit ProjectCard(int projectId, QWidget* parent = nullptr);

    int projectId() const { return m_projectId; }

    void setName(const QString& name);
    void setDescription(const QString& text);   // 2-line truncation
    void setPriority(const QString& priority);  // colors the badge
    void setDeadline(const QDate& deadline);    // computes "Xd left" badge & color
    void setProgress(int pct);
    void setTaskCount(int done, int total);
    void setTeamSize(int n);

signals:
    void clicked(int projectId);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setupUi();
    void applyPriorityStyle(const QString& priority);
    void applyDeadlineStyle(const QDate& deadline);

    int m_projectId = -1;

    QLabel*       m_nameLabel        = nullptr;
    QLabel*       m_descLabel        = nullptr;
    QLabel*       m_priorityBadge    = nullptr;
    QLabel*       m_deadlineBadge    = nullptr;
    QProgressBar* m_progressBar      = nullptr;
    QLabel*       m_taskCountLabel   = nullptr;
    QLabel*       m_teamSizeLabel    = nullptr;
};