#pragma once

#include <QFrame>
#include <QString>
#include <QDate>

class QLabel;

// ============================================================
//  TaskBoardCard — Phase 10 (Projects tasks board)
//
//  One kanban card. Renders a single SessionTaskData row for the
//  Projects detail page. Backing store is unchanged — this widget
//  reads `name`, `unitName`, `status`, `dueDate` from the existing
//  SessionsTasks row.
//
//  Visual mapping (see design/projects-page-mockup.html):
//    - left-edge stripe colored by the *project's* priority (set
//      by the parent board, since per-task priority is out of scope)
//    - title (single line, elided by the parent column width)
//    - unit-name chip
//    - due-date pill (red <=3d, amber <=7d, gray otherwise / "done")
//
//  Right-click opens a small context menu to move the task between
//  status columns or delete it. The board widget wires the menu's
//  actions back to DatabaseManager.
// ============================================================
class TaskBoardCard : public QFrame {
    Q_OBJECT
public:
    explicit TaskBoardCard(int taskId, QWidget* parent = nullptr);

    void setTitle      (const QString& title);
    void setUnitName   (const QString& unit);
    void setStatus     (const QString& status);
    void setDueDate    (const QDate&   due);
    // "high" | "medium" | "low" — controls the left-edge stripe color
    void setPriority   (const QString& priority);
    // First non-empty line of the task description (elided in the UI).
    // Hidden when empty so cards stay slim.
    void setDescription(const QString& description);

    int taskId() const { return m_taskId; }

signals:
    void clicked(int taskId);
    void statusChangeRequested(int taskId, const QString& newStatus);
    void deleteRequested(int taskId);

protected:
    void mousePressEvent (QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void enterEvent      (QEnterEvent* event) override;
    void leaveEvent      (QEvent*      event) override;

private:
    void setupUi();
    void applyStripeColor();
    void refreshDuePill();

    int     m_taskId   = -1;
    QString m_status   = "todo";
    QString m_priority = "medium";
    QDate   m_dueDate;

    QLabel* m_titleLabel = nullptr;
    QLabel* m_descLabel  = nullptr;
    QLabel* m_unitChip   = nullptr;
    QLabel* m_duePill    = nullptr;
};
