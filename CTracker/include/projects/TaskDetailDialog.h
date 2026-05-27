#pragma once

#include <QDialog>
#include "core/DataStructures.h"

class QLineEdit;
class QPlainTextEdit;
class QComboBox;
class QDateEdit;
class QCheckBox;
class QLabel;

// ============================================================
//  TaskDetailDialog — Phase 10.1 (task editor)
//
//  Modal opened when the user clicks a TaskBoardCard. Lets the
//  user edit title, status, due date, and free-form description
//  for a single task. All writes go through DatabaseManager so
//  the board refresh is driven by the existing dataChanged signal.
//
//  Construction takes a task ID, loads it on show, and writes
//  *only the deltas* back on accept — touching DB rows that didn't
//  change creates noise in the activity stream.
// ============================================================
class TaskDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit TaskDetailDialog(int taskId, QWidget* parent = nullptr);

private slots:
    void onAccept();
    void onClearDueDate();

private:
    void buildUi();
    void loadFromDb();

    int             m_taskId   = -1;
    SessionTaskData m_original;            // baseline for delta writes

    QLineEdit*      m_titleEdit   = nullptr;
    QComboBox*      m_statusCombo = nullptr;
    QDateEdit*      m_dueDate     = nullptr;
    QCheckBox*      m_dueEnabled  = nullptr;
    QPlainTextEdit* m_descEdit    = nullptr;
    QLabel*         m_unitLabel   = nullptr;   // read-only display
};
