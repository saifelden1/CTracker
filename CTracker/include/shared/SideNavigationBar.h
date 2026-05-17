#pragma once

#include <QWidget>
#include <QList>

class QPushButton;
class QButtonGroup;

// Task 7.1: SideNavigationBar — 256 px fixed width, 7 nav buttons,
// header (CTracker logo + name), footer (user profile chip).
// Active button uses left accent border in primary green.
class SideNavigationBar : public QWidget {
    Q_OBJECT
public:
    enum Page {
        HomePage      = 0,
        CoursesPage   = 1,
        ProjectsPage  = 2,
        TodoPage      = 3,
        PomodoroPage  = 4,
        AnalyticsPage = 5,
        SettingsPage  = 6,
        PageCount     = 7
    };

    explicit SideNavigationBar(QWidget* parent = nullptr);

    void setActiveButton(int index);
    int  activeButton() const { return m_currentIndex; }

signals:
    void navigationRequested(int pageIndex);

private:
    void setupUi();
    QPushButton* makeNavButton(const QString& iconPath, const QString& activeIconPath, const QString& label, const QString& tooltip);

    QWidget*            m_header       = nullptr;
    QWidget*            m_footer       = nullptr;
    QList<QPushButton*> m_buttons;
    QButtonGroup*       m_group        = nullptr;
    int                 m_currentIndex = HomePage;
};
