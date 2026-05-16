#include "shared/MainWindow.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QFile>
#include <QTextStream>

#include "shared/SideNavigationBar.h"
#include "shared/HomeDashboard.h"
#include "courses/CourseDetailView.h"
#include "projects/ProjectDetailView.h"
#include "analytics/AnalyticsView.h"
#include "settings/SettingsView.h"
#include "analytics/ActivityLogModel.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setObjectName("mainWindow");
    setWindowTitle(tr("CTracker"));
    setMinimumSize(900, 600);
    setupUi();
    setupConnections();
    loadStyleSheet();
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* layout  = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_sideNav = new SideNavigationBar(central);

    m_activityModel = new ActivityLogModel(this);

    m_stack = new QStackedWidget(central);
    m_home          = new HomeDashboard(m_stack);
    m_courseDetail  = new CourseDetailView(m_stack);
    m_projectDetail = new ProjectDetailView(m_stack);
    m_analytics     = new AnalyticsView(m_activityModel, m_stack);
    m_settings      = new SettingsView(m_stack);

    m_stack->insertWidget(HomeStack,      m_home);
    m_stack->insertWidget(CourseStack,    m_courseDetail);
    m_stack->insertWidget(ProjectStack,   m_projectDetail);
    m_stack->insertWidget(AnalyticsStack, m_analytics);
    m_stack->insertWidget(SettingsStack,  m_settings);
    m_stack->setCurrentIndex(HomeStack);

    layout->addWidget(m_sideNav);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);
}

void MainWindow::setupConnections() {
    // The sidebar exposes 5 buttons (Home, Courses, Projects, Analytics, Settings).
    // Base Phase 6 maps Courses & Projects buttons to the matching detail views,
    // showing the last loaded entity (or the empty state). Phase 6 expansion
    // replaces them with CoursesView / ProjectsView list pages.
    connect(m_sideNav, &SideNavigationBar::navigationRequested,
            this, &MainWindow::onNavigationRequested);

    connect(m_home, &HomeDashboard::courseSelected,
            this, &MainWindow::onCourseSelected);
    connect(m_home, &HomeDashboard::projectSelected,
            this, &MainWindow::onProjectSelected);

    connect(m_courseDetail,  &CourseDetailView::backRequested,
            this, &MainWindow::onDetailBackRequested);
    connect(m_projectDetail, &ProjectDetailView::backRequested,
            this, &MainWindow::onDetailBackRequested);
}

void MainWindow::loadStyleSheet() {
    QFile file(QStringLiteral(":/styles/dark-industrial.qss"));
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream in(&file);
    const QString qss = in.readAll();
    if (!qss.isEmpty()) {
        setStyleSheet(qss);
    }
}

void MainWindow::onNavigationRequested(int pageIndex) {
    switch (pageIndex) {
        case SideNavigationBar::HomePage:
            m_stack->setCurrentIndex(HomeStack);
            break;
        case SideNavigationBar::CoursesPage:
            // Base Phase 6: jump to last-loaded course detail (Home if none).
            m_stack->setCurrentIndex(
                m_courseDetail->currentEntityId() >= 0 ? CourseStack : HomeStack);
            break;
        case SideNavigationBar::ProjectsPage:
            m_stack->setCurrentIndex(
                m_projectDetail->currentEntityId() >= 0 ? ProjectStack : HomeStack);
            break;
        case SideNavigationBar::AnalyticsPage:
            m_stack->setCurrentIndex(AnalyticsStack);
            break;
        case SideNavigationBar::SettingsPage:
            m_stack->setCurrentIndex(SettingsStack);
            break;
        default:
            break;
    }
}

void MainWindow::onCourseSelected(int courseId) {
    m_courseDetail->loadCourse(courseId);
    m_stack->setCurrentIndex(CourseStack);
    m_sideNav->setActiveButton(SideNavigationBar::CoursesPage);
}

void MainWindow::onProjectSelected(int projectId) {
    m_projectDetail->loadProject(projectId);
    m_stack->setCurrentIndex(ProjectStack);
    m_sideNav->setActiveButton(SideNavigationBar::ProjectsPage);
}

void MainWindow::onDetailBackRequested() {
    m_stack->setCurrentIndex(HomeStack);
    m_sideNav->setActiveButton(SideNavigationBar::HomePage);
}
