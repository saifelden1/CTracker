#pragma once

#include <QMainWindow>

class SideNavigationBar;
class HomeDashboard;
class CourseDetailView;
class ProjectDetailView;
class AnalyticsView;
class SettingsView;
class ActivityLogModel;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onNavigationRequested(int pageIndex);
    void onCourseSelected(int courseId);
    void onProjectSelected(int projectId);
    void onDetailBackRequested();

private:
    enum StackIndex {
        HomeStack      = 0,
        CourseStack    = 1,
        ProjectStack   = 2,
        AnalyticsStack = 3,
        SettingsStack  = 4,
        StackCount     = 5
    };

    void setupUi();
    void setupConnections();
    void loadStyleSheet();

    SideNavigationBar*  m_sideNav        = nullptr;
    QStackedWidget*     m_stack          = nullptr;
    HomeDashboard*      m_home           = nullptr;
    CourseDetailView*   m_courseDetail   = nullptr;
    ProjectDetailView*  m_projectDetail  = nullptr;
    AnalyticsView*      m_analytics      = nullptr;
    SettingsView*       m_settings       = nullptr;
    ActivityLogModel*   m_activityModel  = nullptr;
};
