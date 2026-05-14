#pragma once

#include <QWidget>
#include <QList>

class QPushButton;
class QButtonGroup;

class SideNavigationBar : public QWidget {
    Q_OBJECT
public:
    enum Page {
        HomePage      = 0,
        CoursesPage   = 1,
        ProjectsPage  = 2,
        AnalyticsPage = 3,
        SettingsPage  = 4,
        PageCount     = 5
    };

    explicit SideNavigationBar(QWidget* parent = nullptr);

    void setActiveButton(int index);
    int  activeButton() const { return m_currentIndex; }

signals:
    void navigationRequested(int pageIndex);

private:
    void setupButtons();
    QPushButton* makeNavButton(const QString& glyph, const QString& tooltip);

    QList<QPushButton*> m_buttons;
    QButtonGroup*       m_group        = nullptr;
    int                 m_currentIndex = HomePage;
};
