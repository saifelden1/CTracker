#pragma once

#include <QWidget>
#include <QMap>
#include <QString>

class QLabel;
class QPushButton;
class QVBoxLayout;
class SessionTaskRow;

class UnitExpandableWidget : public QWidget {
    Q_OBJECT
public:
    explicit UnitExpandableWidget(int unitId,
                                  const QString& name,
                                  QWidget* parent = nullptr);

    int     unitId()   const { return m_unitId; }
    QString name()     const { return m_name; }
    bool    isExpanded() const { return m_expanded; }

    void setExpanded(bool expanded);
    void setName(const QString& newName);

    void addSessionTask(int sessionId, const QString& name, int progress);
    void removeSessionTask(int sessionId);
    void updateSessionTaskProgress(int sessionId, int progress);

    int  calculateOverallProgress() const;

signals:
    void sessionTaskProgressChanged(int sessionId, int oldValue, int newValue);
    void sessionTaskRenamed(int sessionId, const QString& newName);
    void expandStateChanged(bool expanded);

private slots:
    void onExpandClicked();
    void onChildProgressChanged(int oldValue, int newValue);
    void onChildNameChanged(const QString& newName);

private:
    void setupUi();
    void refreshOverall();

    int     m_unitId   = -1;
    QString m_name;
    bool    m_expanded = false;

    QPushButton* m_expandButton  = nullptr;
    QLabel*      m_nameLabel     = nullptr;
    QLabel*      m_progressLabel = nullptr;
    QWidget*     m_content       = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;

    QMap<int, SessionTaskRow*> m_rows;
};
