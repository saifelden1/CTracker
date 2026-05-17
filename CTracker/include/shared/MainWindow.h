#pragma once

#include <QMainWindow>

class SideNavigationBar;
class HomeDashboard;
class CoursesView;
class ProjectsView;
class TodoView;
class PomodoroView;
class CourseDetailView;
class ProjectDetailView;
class AnalyticsView;
class SettingsView;
class QStackedWidget;

// Task 7.12: MainWindow — QStackedWidget with all 7 pages + 2 hidden detail pages.
// Home (0), Courses (1), Projects (2), To-Do (3), Pomodoro (4), Analytics (5), Settings (6)
// Plus 2 hidden detail pages (CourseDetail, ProjectDetail) reached via card clicks.
// Wire SideNavigationBar::navigationRequested → setCurrentIndex.
// Wire HomeDashboard::courseSelected → CourseDetailView::loadCourse → switch view.
// Same for projectSelected.
// loadStyleSheet(":/styles/dark-industrial.qss").
// Min size 1280×800; persist geometry via QSettings.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onNavigationRequested(int pageIndex);
    void onCourseSelected(int courseId);
    void onProjectSelected(int projectId);
    void onDetailBackRequested();
    void onAddNewCourseRequested();
    void onAddNewProjectRequested();

private:
    enum StackIndex {
        HomeStack      = 0,
        CoursesStack   = 1,
        ProjectsStack  = 2,
        TodoStack      = 3,
        PomodoroStack  = 4,
        AnalyticsStack = 5,
        SettingsStack  = 6,
        CourseDetailStack = 7,
        ProjectDetailStack = 8,
        StackCount     = 9
    };

    void setupUi();
    void setupConnections();
    void loadStyleSheet();

    SideNavigationBar*  m_sideNav        = nullptr;
    QStackedWidget*     m_stack          = nullptr;
    HomeDashboard*      m_home           = nullptr;
    CoursesView*        m_courses        = nullptr;
    ProjectsView*       m_projects       = nullptr;
    TodoView*           m_todo           = nullptr;
    PomodoroView*       m_pomodoro       = nullptr;
    CourseDetailView*   m_courseDetail   = nullptr;
    ProjectDetailView*  m_projectDetail  = nullptr;
    AnalyticsView*      m_analytics      = nullptr;
    SettingsView*       m_settings       = nullptr;
};
