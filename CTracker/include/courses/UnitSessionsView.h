#pragma once

#include <QWidget>
#include <QString>
#include <QMap>

class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class CircularProgressBar;
class SessionTaskRow;

// Task 7.5a — UnitSessionsView
//
// Focused per-unit sessions view shown when the user clicks a UnitCard
// in CourseDetailView's units grid. Lives inside the inner
// QStackedWidget as Page 1 (Page 0 is the units grid).
//
// Layout:
//   ┌──────────────────────────────────────────────────────────┐
//   │ [↩ Units]  <unit name>           ◯  [+ Session] [Rename] [Delete] │
//   ├──────────────────────────────────────────────────────────┤
//   │  QScrollArea                                              │
//   │     SessionTaskRow                                        │
//   │     SessionTaskRow                                        │
//   │     SessionTaskRow                                        │
//   │     …                                                     │
//   └──────────────────────────────────────────────────────────┘
//
// Owns its SessionTaskRow widgets in a QMap<int, SessionTaskRow*>
// keyed by sessionId for O(1) updates — same pattern the old
// UnitExpandableWidget used internally.
class UnitSessionsView : public QWidget {
    Q_OBJECT
public:
    explicit UnitSessionsView(QWidget* parent = nullptr);

    void loadUnit(int unitId);
    int  currentUnitId() const { return m_unitId; }

signals:
    void backRequested();
    void sessionProgressChanged(int sessionId, int oldValue, int newValue);
    void sessionRenamed(int sessionId, const QString& newName);
    void sessionAdded(int unitId);
    void unitRenamed(int unitId, const QString& newName);
    void unitRemoved(int unitId);

public slots:
    // Re-fetches the current unit's sessions from the DB and rebuilds
    // the rows. Safe to call when the unit has been removed externally
    // (clears state + emits unitRemoved). No-op when m_unitId < 0.
    void refresh();

private slots:
    void onAddSessionClicked();
    void onRenameUnitClicked();
    void onDeleteUnitClicked();
    void onChildProgressChanged(int oldValue, int newValue);
    void onChildNameChanged(const QString& newName);

private:
    void setupUi();
    void clearRows();
    void rebuildRows();
    void refreshRing();
    bool fetchUnitName(QString* outName) const;   // returns false if unit gone

    int     m_unitId = -1;
    QString m_unitName;

    QPushButton*         m_backBtn       = nullptr;
    QLabel*              m_titleLabel    = nullptr;
    CircularProgressBar* m_ring          = nullptr;
    QPushButton*         m_addSessionBtn = nullptr;
    QPushButton*         m_renameUnitBtn = nullptr;
    QPushButton*         m_deleteUnitBtn = nullptr;

    QScrollArea*  m_scrollArea    = nullptr;
    QWidget*      m_rowsContainer = nullptr;
    QVBoxLayout*  m_rowsLayout    = nullptr;

    QMap<int, SessionTaskRow*> m_rows;
};
