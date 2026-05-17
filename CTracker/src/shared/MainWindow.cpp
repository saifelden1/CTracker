#include "shared/MainWindow.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>

#include "shared/SideNavigationBar.h"
#include "shared/HomeDashboard.h"
#include "shared/EntityCreateDialog.h"
#include "courses/CoursesView.h"
#include "courses/CourseDetailView.h"
#include "projects/ProjectsView.h"
#include "projects/ProjectDetailView.h"
#include "todos/TodoView.h"
#include "pomodoro/PomodoroView.h"
#include "analytics/AnalyticsView.h"
#include "settings/SettingsView.h"

// Task 7.12: MainWindow — 7 main pages + 2 hidden detail pages.
// Min size 1280×800; persist geometry via QSettings.

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setObjectName("mainWindow");
    setWindowTitle(tr("CTracker"));
    setMinimumSize(1280, 800);

    // Force window flags to ensure it shows
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | 
                   Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | 
                   Qt::WindowCloseButtonHint);

    // Restore saved geometry
    QSettings settings;
    const QSize savedSize = settings.value("mainWindow/size", QSize(1280, 800)).toSize();
    resize(savedSize);

    setupUi();
    setupConnections();
    loadStyleSheet();
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* layout  = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    qDebug() << "Creating SideNavigationBar...";
    m_sideNav = new SideNavigationBar(central);

    qDebug() << "Creating QStackedWidget...";
    m_stack = new QStackedWidget(central);

    // 7 main pages in order matching SideNavigationBar::Page enum
    qDebug() << "Creating HomeDashboard...";
    m_home           = new HomeDashboard(m_stack);
    qDebug() << "Creating CoursesView...";
    m_courses        = new CoursesView(m_stack);
    qDebug() << "Creating ProjectsView...";
    m_projects       = new ProjectsView(m_stack);
    qDebug() << "Creating TodoView...";
    m_todo           = new TodoView(m_stack);
    qDebug() << "Creating PomodoroView...";
    m_pomodoro       = new PomodoroView(m_stack);
    qDebug() << "Creating AnalyticsView...";
    m_analytics      = new AnalyticsView(m_stack);
    qDebug() << "Creating SettingsView...";
    m_settings       = new SettingsView(m_stack);

    // 2 hidden detail pages (reached via card clicks)
    qDebug() << "Creating CourseDetailView...";
    m_courseDetail   = new CourseDetailView(m_stack);
    qDebug() << "Creating ProjectDetailView...";
    m_projectDetail  = new ProjectDetailView(m_stack);

    // Insert in StackIndex order
    m_stack->insertWidget(HomeStack,         m_home);
    m_stack->insertWidget(CoursesStack,      m_courses);
    m_stack->insertWidget(ProjectsStack,     m_projects);
    m_stack->insertWidget(TodoStack,         m_todo);
    m_stack->insertWidget(PomodoroStack,     m_pomodoro);
    m_stack->insertWidget(AnalyticsStack,    m_analytics);
    m_stack->insertWidget(SettingsStack,     m_settings);
    m_stack->insertWidget(CourseDetailStack, m_courseDetail);
    m_stack->insertWidget(ProjectDetailStack, m_projectDetail);
    m_stack->setCurrentIndex(HomeStack);

    layout->addWidget(m_sideNav);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);
}

void MainWindow::setupConnections() {
    // ── Navigation ──────────────────────────────────────────
    connect(m_sideNav, &SideNavigationBar::navigationRequested,
            this, &MainWindow::onNavigationRequested);

    // ── Home dashboard → detail views ───────────────────────
    connect(m_home, &HomeDashboard::courseSelected,
            this, &MainWindow::onCourseSelected);
    connect(m_home, &HomeDashboard::projectSelected,
            this, &MainWindow::onProjectSelected);

    // ── Courses list → course detail ────────────────────────
    connect(m_courses, &CoursesView::courseSelected,
            this, &MainWindow::onCourseSelected);

    // ── Projects list → project detail ──────────────────────
    connect(m_projects, &ProjectsView::projectSelected,
            this, &MainWindow::onProjectSelected);

    // ── "Add New" buttons → EntityCreateDialog ──────────────
    connect(m_courses, &CoursesView::addNewRequested,
            this, &MainWindow::onAddNewCourseRequested);
    connect(m_projects, &ProjectsView::addNewRequested,
            this, &MainWindow::onAddNewProjectRequested);

    // ── Detail view back buttons ────────────────────────────
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
    // pageIndex maps 1:1 to SideNavigationBar::Page enum and StackIndex
    m_stack->setCurrentIndex(pageIndex);
}

void MainWindow::onCourseSelected(int courseId) {
    m_courseDetail->loadCourse(courseId);
    m_stack->setCurrentIndex(CourseDetailStack);
    m_sideNav->setActiveButton(SideNavigationBar::CoursesPage);
}

void MainWindow::onProjectSelected(int projectId) {
    m_projectDetail->loadProject(projectId);
    m_stack->setCurrentIndex(ProjectDetailStack);
    m_sideNav->setActiveButton(SideNavigationBar::ProjectsPage);
}

void MainWindow::onDetailBackRequested() {
    // Return to the list page (Courses or Projects) depending on which detail was shown
    const int currentIndex = m_stack->currentIndex();
    if (currentIndex == CourseDetailStack) {
        m_stack->setCurrentIndex(CoursesStack);
    } else if (currentIndex == ProjectDetailStack) {
        m_stack->setCurrentIndex(ProjectsStack);
    } else {
        m_stack->setCurrentIndex(HomeStack);
    }
    // Keep the sidebar button matching the page we returned to
    const int newIndex = m_stack->currentIndex();
    m_sideNav->setActiveButton(newIndex);
}

void MainWindow::onAddNewCourseRequested() {
    EntityCreateDialog dlg(EntityCreateDialog::Mode::CourseOrProject, this);
    if (dlg.exec() == QDialog::Accepted) {
        const int id = dlg.createdEntityId();
        if (id >= 0) {
            if (dlg.entityType() == "course") {
                onCourseSelected(id);
            } else {
                onProjectSelected(id);
            }
        }
    }
}

void MainWindow::onAddNewProjectRequested() {
    EntityCreateDialog dlg(EntityCreateDialog::Mode::ProjectOnly, this);
    if (dlg.exec() == QDialog::Accepted) {
        const int id = dlg.createdEntityId();
        if (id >= 0) {
            onProjectSelected(id);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Persist window geometry via QSettings
    QSettings settings;
    settings.setValue("mainWindow/size", size());
    settings.setValue("mainWindow/pos", pos());
    event->accept();
}
